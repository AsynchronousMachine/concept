#pragma once

#include <functional>
#include <variant>

#include "../maker/maker_do.hpp"

namespace Asm {
	// Template class for a link
	template <typename D1, typename D2>
	class LinkObject
	{
	public:
		// Callback function can be defined everywhere
		using cb_type = std::function<void(D1&, D2&)>;

	private:
		cb_type _cb;

	public:
		template <typename MemFun, typename ThisPtr>
		LinkObject(MemFun memfun, ThisPtr thisptr) : _cb([thisptr, memfun](D1& d1, D2& d2) { std::mem_fn(memfun)(thisptr, d1, d2); }) {}
		
		LinkObject(cb_type cb) : _cb(cb) {}
		
		// Only important for std::variant; don't use this constructor: linking is not possible!
		LinkObject() : _cb(nullptr) {}

		// Non-copyable
		LinkObject(const LinkObject&) = delete;
		LinkObject &operator=(const LinkObject&) = delete;

		// Non-movable
		LinkObject(LinkObject&&) = delete;
		LinkObject &operator=(LinkObject&&) = delete;

		// Necessary if someone want to inherit from that
		virtual ~LinkObject() = default;

		void set(const std::string& name, const Asm::data_variant& a1, const Asm::data_variant& a2) {
			  if (_cb == nullptr)
				    return;

			  std::get<D1*>(a1)->registerLink(name, *std::get<D2*>(a2), _cb);
		}

        void set(const std::string& name, D1& d1, D2& d2) {
            if (_cb == nullptr)
                return;

            d1.registerLink(name, d2, _cb);
        }

		void clear(const std::string& name, const Asm::data_variant& a) {
			  std::visit([&](auto d1){ d1->unregisterLink(name); }, a);
		}

        void clear(const std::string& name, D1& d1) {
            d1.unregisterLink(name);
        }
	};
}
