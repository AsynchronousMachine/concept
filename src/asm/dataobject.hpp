#pragma once

#include <type_traits>
#include <functional>
#include <unordered_map>
#include <atomic>

#include <boost/thread/mutex.hpp>
#include <boost/thread/null_mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/shared_lock_guard.hpp>
#include <boost/variant.hpp>

#include "../../external/rapidjson/include/rapidjson/document.h"
#include "dataobjectreactor.hpp"

namespace Asm {

	template<typename DOtype, typename type>
	using enable_if_is_same = std::enable_if_t<std::is_same<DOtype, type>::value, bool>;

	constexpr bool default_serializer = true;

	using serializeFnct = std::function<void(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator)>;
	using deserializeFnct = std::function<void(rapidjson::Value& value)>;

	// Concept of dataobject
	//
	// Dataobjects (aka DO) are container to hold data (content) and callback functions (aka LINKS) as notifier
	// DOs can be linked together via LINKS
	// DOs can be locked and unlocked to handle there content consistently
	// The associated callback functions (LINKS) are called if the content of the original DO has been changed
	//
	// DO1 ------LINK1-------> DO2    DO1 is linked to DO2
	//        |
	//        +--LINK3-------> DO3    Also DO1 is linked to DO3

	// Template class for a dataobject
	template <typename D>
	class DataObject
	{
		static_assert(!std::is_void<D>::value, "DataObjects don't support void");

		friend class DataObjectReactor; // This enables the reactor to traverse the links from outside

	private:
		using content_t = std::conditional_t<std::is_arithmetic<D>::value, std::atomic<D>, D>;
		using mutex_t = std::conditional_t<std::is_arithmetic<D>::value, boost::null_mutex, boost::shared_mutex>;

		// Content for this DO
		content_t _content;

		// Mutable member as it needs to be modified in the const member function get()
		mutable mutex_t _mtx_content;

		// This holds all callbacks (LINKS) linked to that DO
		std::unordered_map<std::string, std::function<void()>> _links;

		// Protect the map of linked DOs
		boost::mutex _mtx_links;

		inline D get() const
		{
			// Shared lock to support concurrent access from multiple visitors in different threads
			boost::shared_lock_guard<mutex_t> lock(_mtx_content);
			return _content;
		}

		inline void set(D d)
		{
			// Shared lock to support concurrent access from multiple visitors in different threads
			boost::shared_lock_guard<mutex_t> lock(_mtx_content);
			_content = d;
		}

		serializeFnct doSerialize;

		template< class  U = D, enable_if_is_same<U, bool> = true>
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
		DataObject(D content, bool b) : _content(content), doSerialize([this](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) { std::mem_fn(&DataObject<D>::serialize_impl<D>)(this, value, allocator); }), doDeserialize([this](rapidjson::Value& value) { std::mem_fn(&DataObject<D>::deserialize_impl<D>)(this, value); }) {}
		DataObject(D content, serializeFnct ptr, deserializeFnct ptr2) : _content(content), doSerialize(ptr), doDeserialize(ptr2) {}
		DataObject(D content) : _content(content), doSerialize([](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {}), doDeserialize([](rapidjson::Value& value) {}) {}
		template <typename MemFun, typename MemFun2, typename ThisPtr>
		DataObject(D content, MemFun ser, MemFun2 deser, ThisPtr thisptr) : _content(content), doSerialize([thisptr, ser](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) { std::mem_fn(ser)(thisptr, value, allocator); }), doDeserialize([thisptr, deser](rapidjson::Value& value) { std::mem_fn(deser)(thisptr, value); }) { std::cout << "### A ###" << std::endl; }
		template <typename MemFun, typename MemFun2>
		DataObject(D content, MemFun ser, MemFun2 deser, bool b) : _content(content), doSerialize([&, ser](rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) { std::mem_fn(ser)(&_content, value, allocator); }), doDeserialize([&, deser](rapidjson::Value& value) { std::mem_fn(deser)(&_content, value); }) { std::cout << "### B ###" << std::endl; }

		// Non-copyable
		DataObject(const DataObject&) = delete;
		DataObject &operator=(const DataObject&) = delete;

		// Non-movable
		DataObject(DataObject&&) = delete;
		DataObject &operator=(DataObject&&) = delete;

		// Necessary if someone want to inherit from that
		virtual ~DataObject() = default;

		template <class Visitor>
		void set(Visitor visitor)
		{
			// Exclusive lock for write access
			boost::lock_guard<mutex_t> lock(_mtx_content);
			visitor(_content);
		}

		template <class Visitor>
		void setAndTrigger(Visitor visitor, Asm::DataObjectReactor& reactor)
		{
			set(visitor);
			reactor.trigger(*this);
		}

		// Const member function to avoid that a non-const reference is passed to the visitor
		// as that would allow the visitor to change the data_ member
		template <class Visitor>
		auto get(Visitor visitor) const
		{
			// Shared lock to support concurrent access from multiple visitors in different threads
			boost::shared_lock_guard<mutex_t> lock(_mtx_content);
			return visitor(_content);
		}

		// Link a DO to that DO
		template <typename D2, typename CB>
		void registerLink(const std::string name, DataObject<D2>& d2, CB cb)
		{
			boost::lock_guard<boost::mutex> lock(_mtx_links);
			_links.insert({ name, [cb, this, &d2] { cb(*this, d2); } });
			std::cout << "registerLink: " << name << std::endl;
		}

		// Remove a link to that DO by name
		void unregisterLink(const std::string name)
		{
			boost::lock_guard<boost::mutex> lock(_mtx_links);
			_links.erase(name);
			std::cout << "UNregisterLink: " << name << std::endl;
		}


		void serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator)
		{
			return doSerialize(value, allocator);
		}

		void deserialize(rapidjson::Value& value)
		{
			doDeserialize(value);
		}
	};

	struct EmptyDataobject : boost::blank
	{
		const std::string serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) { return "!!!Will never be used!!!"; };
		const void deserialize(rapidjson::Value& value) {};
		template <class Visitor>
		void set(Visitor visitor) {};
		template <class Visitor>
		void setAndTrigger(Visitor visitor, Asm::DataObjectReactor& reactor) {};
		template <class Visitor>
		auto get(Visitor visitor) { return 0; }; // !!!Will never be used!!!
		template <typename D2, typename CB>
		void registerLink(const std::string name, DataObject<D2>& d2, CB cb) {};
		void unregisterLink(const std::string name) {};
	};
}
