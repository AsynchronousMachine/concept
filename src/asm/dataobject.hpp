/*
** DataObjects (aka DO) are container to hold data (content),
** make it accessible via a visitor idiom for consistency and
** stores callback functions (aka LINKS) to let it communicate to each other
**
** DO1 ------LINK1-------> DO2    DO1 is linked to DO2
**  |
**  +--------LINK3-------> DO3    Also DO1 is linked to DO3
*/

#pragma once

#include <type_traits>
#include <functional>
#include <unordered_map>
#include <atomic>

#include <boost/core/demangle.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/null_mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/shared_lock_guard.hpp>
#include <boost/variant.hpp>

#include "../../external/rapidjson/include/rapidjson/document.h"

#include "dataobjectreactor.hpp"

namespace Asm {
constexpr bool default_serializer = true;

template<typename DOtype, typename type>
using enable_if_is_same = std::enable_if_t<std::is_same<DOtype, type>::value, bool>;

using serializeFnct = std::function<void(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator)>;
using deserializeFnct = std::function<void(rapidjson::Value& value)>;

template <typename D>
class DataObject {
    static_assert(!std::is_void<D>::value, "DataObjects don't support void");

    friend class DataObjectReactor; // This enables the reactor to traverse the links from outside

  private:
    using content_t = std::conditional_t<std::is_arithmetic<D>::value, std::atomic<D>, D>;
    using mutex_t = std::conditional_t<std::is_arithmetic<D>::value, boost::null_mutex, boost::shared_mutex>;

    // Content for this DO
    content_t _content;

    // Mutable member as it needs to be modified in the const member function get()
    mutable mutex_t _mtx_content;

    // This holds all callback functions (LINKS) linked to that DO
    std::unordered_map<std::string, std::function<void()>> _links;

    // Protect the map of linked DOs
    boost::mutex _mtx_links;

    inline D get() const {
        // Shared lock to support concurrent access from multiple visitors in different threads
        boost::shared_lock_guard<mutex_t> lock(_mtx_content);
        return _content;
    }

    inline void set(D d) {
        // Shared lock to support concurrent access from multiple visitors in different threads
        boost::shared_lock_guard<mutex_t> lock(_mtx_content);
        _content = d;
    }

    serializeFnct doSerialize;

    template <typename U = D, enable_if_is_same<U, bool> = true>
    void serialize_impl(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
        value.SetBool(get());
    }

    template< typename U = D, enable_if_is_same<U, int> = true>
    void serialize_impl(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
        value.SetInt(get());
    }

    template< typename U = D, enable_if_is_same<U, double> = true>
    void serialize_impl(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
        value.SetDouble(get());
    }

    template< typename U = D, enable_if_is_same<U, std::string> = true>
    void serialize_impl(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
        value.SetString(get().c_str(), allocator);
    }

    template< class U = D, std::enable_if_t<
                  !std::is_same<U, bool>::value &&
                  !std::is_same<U, int>::value &&
                  !std::is_same<U, double>::value &&
                  !std::is_same<U, std::string>::value, bool> = true>
    void serialize_impl(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) { }

    deserializeFnct doDeserialize;

    template< class  U = D, enable_if_is_same<U, bool> = true>
    void deserialize_impl(rapidjson::Value& value) {
        set(value.GetBool());
    }

    template< typename U = D, enable_if_is_same<U, int> = true>
    void deserialize_impl(rapidjson::Value& value) {
        set(value.GetInt());
    }

    template< typename U = D, enable_if_is_same<U, double> = true>
    void deserialize_impl(rapidjson::Value& value) {
        set(value.GetDouble());
    }

    template< typename U = D, enable_if_is_same<U, std::string> = true>
    void deserialize_impl(rapidjson::Value& value) {
        std::string s{ value.GetString() };
        set(s);
    }

    template< class U = D, std::enable_if_t<
                  !std::is_same<U, bool>::value &&
                  !std::is_same<U, int>::value &&
                  !std::is_same<U, double>::value &&
                  !std::is_same<U, std::string>::value, bool> = true>
    void deserialize_impl(rapidjson::Value& value) { }

  public:
    DataObject(D content, bool b) : _content(content),
                                    doSerialize([this](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
                                        std::mem_fn(&DataObject<D>::serialize_impl<D>)(this, value, allocator); }),
                                    doDeserialize([this](rapidjson::Value& value) {
                                        std::mem_fn(&DataObject<D>::deserialize_impl<D>)(this, value); }) {
        std::cout << "DO-CTOR with implicit trival ser-/deser fct" << std::endl;
    }

    DataObject(D content, serializeFnct ptr, deserializeFnct ptr2) : _content(content),
                                                                     doSerialize(ptr),
                                                                     doDeserialize(ptr2) {
        std::cout << "DO-CTOR with explicit ser-/deser fct" << std::endl;
    }

    DataObject(D content) : _content(content),
                            doSerialize([](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {}),
                            doDeserialize([](rapidjson::Value& value) {}) {
        std::cout << "DO-CTOR with disabled ser-/deser" << std::endl;
    }

    template <typename MemFun, typename MemFun2, typename ThisPtr>
    DataObject(D content, MemFun ser, MemFun2 deser, ThisPtr thisptr) : _content(content),
                                                                        doSerialize([thisptr, ser](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
                                                                            std::mem_fn(ser)(thisptr, value, allocator); }),
                                                                        doDeserialize([thisptr, deser](rapidjson::Value& value) {
                                                                            std::mem_fn(deser)(thisptr, value); }) {
        std::cout << "DO-CTOR with instance based ser-/deser fct" << std::endl;
    }

//    template <typename MemFun, typename MemFun2>
//    DataObject(D content, MemFun ser, MemFun2 deser, bool b) : _content(content),
//                                                               doSerialize([&, ser](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
//                                                                   std::mem_fn(ser)(&_content, value, allocator); }),
//                                                               doDeserialize([&, deser](rapidjson::Value& value) {
//                                                                   std::mem_fn(deser)(&_content, value); }) {
//        std::cout << "DO-CTOR with content based ser-/deser fct" << std::endl;
//    }

    template <typename MemFun, typename MemFun2>
    DataObject(D content, MemFun ser, MemFun2 deser, std::enable_if_t<std::is_member_function_pointer<MemFun>::value, bool> = true)
        : _content(content),
          doSerialize([&, ser](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) { std::mem_fn(ser)(&_content, value, allocator); }),
          doDeserialize([&, deser](rapidjson::Value& value) { std::mem_fn(deser)(&_content, value); })
    {
        std::cout << "DO-CTOR with content based ser-/deser fct" << std::endl;
    }

    DataObject() : doSerialize([](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {}),
                   doDeserialize([](rapidjson::Value& value) {}) { std::cout << "DO-CTOR only for TO" << std::endl; }

    // Non-copyable
    DataObject(const DataObject&) = delete;
    DataObject &operator=(const DataObject&) = delete;

    // Non-movable
    DataObject(DataObject&&) = delete;
    DataObject &operator=(DataObject&&) = delete;

    // Necessary if someone want to inherit from that
    virtual ~DataObject() = default;

    template <class Visitor>
    void set(Visitor visitor) {
        // Exclusive lock for write access
        boost::lock_guard<mutex_t> lock(_mtx_content);
        visitor(_content);
    }

    template <class Visitor>
    void setAndTrigger(Visitor visitor, Asm::DataObjectReactor& reactor) {
        set(visitor);
        reactor.trigger(*this);
    }

    // Const member function to avoid that a non-const reference is passed to the visitor
    // as that would allow the visitor to change the data_ member
    template <class Visitor>
    auto get(Visitor visitor) const {
        // Shared lock to support concurrent access from multiple visitors in different threads
        boost::shared_lock_guard<mutex_t> lock(_mtx_content);
        return visitor(_content);
    }

    // Link a DO to that DO by registering a callback function
    template <typename D2, typename LINK>
    void registerLink(const std::string& name, DataObject<D2>& d2, LINK cb) {
        boost::lock_guard<boost::mutex> lock(_mtx_links);
        _links.insert({ name, [cb, this, &d2] { cb(*this, d2); } });
        std::cout << "RegisterLink: " << name << std::endl;
        std::cout << "From " << boost::core::demangle(typeid(*this).name()) << " to " << boost::core::demangle(typeid(d2).name()) << " via " << boost::core::demangle(typeid(cb).name()) << std::endl;
    }

    // Remove a link to that DO by name
    void unregisterLink(const std::string& name) {
        boost::lock_guard<boost::mutex> lock(_mtx_links);
        _links.erase(name);
        std::cout << "UnregisterLink: " << name << std::endl;
        std::cout << "From " << boost::core::demangle(typeid(*this).name()) << std::endl;
    }

    void serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
        return doSerialize(value, allocator);
    }

    void deserialize(rapidjson::Value& value) {
        doDeserialize(value);
    }
};
}
