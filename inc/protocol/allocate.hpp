#pragma once

namespace dictos::net::protocol {

inline std::unique_ptr<class AbstractProtocol> allocateProtocol(Address addr, config::Context &context, AbstractProtocol::ErrorCallback ecb)
{
	switch (addr.protocol())
	{
		case TYPE::TCPv4:
		case TYPE::TCPv6:
			return std::make_unique<Tcp>(std::move(addr), std::forward<config::Context &>(context), std::move(ecb));

		default:
			DCORE_THROW(InvalidArgument, "Protocol:", addr.protocol(), "is not currently supported (when constructing from address:", addr, ")");
	}
}

}
