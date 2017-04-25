#pragma once

#include <functional>

#include <boost/variant.hpp>

#include "maker_do.hpp"


namespace Asm {

	// Template class for a link
	template <typename D1, typename D2>
	class LinkObject
	{
	public:
		//full flexibility for simple linking
		//callback function can be defined everywhere, D2 does not have to be part of a module
		using cb_type = std::function<void(D1&, D2&)>;

	private:
		cb_type _cb;

	public:
		template <typename MemFun, typename ThisPtr>
		LinkObject(MemFun memfun, ThisPtr thisptr) : _cb([thisptr, memfun](D1& d1, D2& d2) { std::mem_fn(memfun)(thisptr, d1, d2); }) {}
		LinkObject(cb_type cb) : _cb(cb) {}

		// Non-copyable
		LinkObject(const LinkObject&) = delete;
		LinkObject &operator=(const LinkObject&) = delete;

		// Non-movable
		LinkObject(LinkObject&&) = delete;
		LinkObject &operator=(LinkObject&&) = delete;

		// Necessary if someone want to inherit from that
		virtual ~LinkObject() = default;

		void set(const std::string name, data_variant a1, data_variant a2)
		{
			D1 &d1 = boost::get<D1&>(a1);
			D2 &d2 = boost::get<D2&>(a2);
			d1.registerLink(name, d2, _cb);

			/*
			TODO Aktuell nicht übersetzbar
			boost::apply_visitor([&](D1& d1) {d1.registerLink(name, d2, _cb); }, d1);
			*/
		}

		void clear(const std::string name, data_variant a)
		{
			D1 &d1 = boost::get<D1&>(a);

			d1.unregisterLink(name);
		}
	};


	struct EmptyLinkObject : boost::blank
	{
		void set(const std::string name, data_variant a1, data_variant a2) {};
		void clear(const std::string name, data_variant a) {}; // !!!Will never be used!!!
	};
}