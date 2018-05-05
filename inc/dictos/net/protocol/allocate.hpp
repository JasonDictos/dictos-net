#pragma once

namespace dictos::net::protocol {

inline std::unique_ptr<class AbstractProtocol> allocateProtocol(Address addr, config::Context &context, AbstractProtocol::ErrorCallback ecb)
{
	switch (addr.protocol())
	{
		case TYPE::Tcp:
			return std::make_unique<Tcp>(std::move(addr), std::forward<config::Context &>(context), std::move(ecb));

		case TYPE::Ssl:
			return std::make_unique<Ssl>(std::move(addr), std::forward<config::Context &>(context), std::move(ecb), SslContext(context.getOptions()));

		case TYPE::WebSocket:
			return std::make_unique<WebSocket>(std::move(addr), std::forward<config::Context &>(context), std::move(ecb));

		case TYPE::SslWebSocket:
			return std::make_unique<SslWebSocket>(std::move(addr), std::forward<config::Context &>(context), std::move(ecb), SslContext(context.getOptions()));

		default:
			DCORE_THROW(InvalidArgument, "Protocol:", addr.protocol(), "is not currently supported (when constructing from address:", addr, ")");
	}
}

}
