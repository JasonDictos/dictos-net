#pragma once

#include <boost/beast/websocket.hpp>

namespace dictos::net::protocol {

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

/**
 * The WebSocket protocol adapts the stream to a websocket.
 */
class WebSocket : public AbstractProtocol
{
public:
	WebSocket(Address addr, config::Context &config, ErrorCallback ecb) :
		AbstractProtocol(std::move(addr), config, std::move(ecb)),
		m_socket(GlobalContext())
	{
	}

	void close() noexcept override
	{
		// @@ TODO
	}

	void accept(ProtocolUPtr &newProtocol, AcceptCallback cb) override
	{
		// @@ TODO
	}

	void read(Size size, ReadCallback cb) const override
	{
		// @@ TODO
	}

	void connect(ConnectCallback cb) override
	{
		// @@ TODO
	}

	void write(memory::Heap payload, WriteCallback cb) override
	{
		// @@ TODO
	}

	websocket::stream<tcp::socket> m_socket;
};

}
