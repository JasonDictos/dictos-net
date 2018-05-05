#pragma once

namespace dictos::net::protocol {

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
namespace ssl = boost::asio::ssl;

/**
 * The WebSocket protocol adapts the stream to a websocket.
 */
class SslWebSocket : public AbstractProtocol
{
public:
	SslWebSocket(Address addr, config::Context &config, ErrorCallback ecb, SslContext sslContext) :
		AbstractProtocol(std::move(addr), config, std::move(ecb)),
		m_socket(m_service),
		m_webSocket(m_socket, sslContext),
		m_sslContext(std::move(sslContext))
	{
	}

	void close() override
	{
		m_webSocket.close(websocket::close_code::normal);
	}

	void accept(ProtocolUPtr &newProtocol, AcceptCallback cb) override
	{
		m_acceptor = std::make_unique<tcp::acceptor>(
			GlobalContext(),
			tcp::endpoint(
				Address::IpAddress::from_string(
					m_localAddress.ip()
				),
				m_localAddress.port()
			)
		);

		// Now issue the accept and bind the lambda to the new protocol
		m_acceptor->async_accept(
			staticUPtrCast<SslWebSocket>(newProtocol)->m_socket.lowest_layer(),
			[this,cb = std::move(cb),&newProtocol](boost::system::error_code ec) mutable
			{
				if (errorCheck<OP::Accept>(ec))
					return;

				// Successfully connected, do handshake
				staticUPtrCast<SslWebSocket>(newProtocol)->m_webSocket.next_layer().async_handshake(ssl::stream_base::server,
					[this,cb = std::move(cb)](boost::system::error_code ec)
					{
						if (errorCheck<OP::SslHandshake>(ec))
							return;

						// Phew finally, call the callers cb
						cb();
					}
				);
			}
		);
	}

	void read(Size size, ReadCallback cb) const override
	{
		// Allocate the buffer to read into for the caller
		memory::Heap result(size);

		// Construct an asio buffer before we move the result into the closure, due to
		// parameter initialization order this prevents a crash since result will
		// get moved before it gets passed into the async_read call.
		boost::asio::mutable_buffer buf(result.cast<void *>(), result.size());

		// Submit the read to the service and bootstrap the callbacks
		boost::asio::async_read(m_webSocket.next_layer(), buf,
			[this,result = std::move(result), cb = std::move(cb)](boost::system::error_code ec, size_t sizeRead)
			{
				if (errorCheck<OP::Read>(ec))
					return;

				DCORE_ASSERT(sizeRead == result.size());
				cb(result);
			}
		);
	}

	void connect(ConnectCallback cb) override
	{
		// First we go through a few hoops to resolve the address
		m_resolver = std::make_unique<tcp::resolver>(m_service);
		m_resolver->async_resolve(tcp::v4(), m_localAddress.ip(), string::toString(m_localAddress.port()),
			[this,cb = std::move(cb)](boost::system::error_code ec, tcp::resolver::iterator results)
			{
				if (errorCheck<OP::Resolve>(ec))
					return;

				// Ok connect for each resolved entry, first one that connects ok will stop the enum
				boost::asio::async_connect(m_webSocket.next_layer().next_layer(), results,
					[this,cb = std::move(cb)](boost::system::error_code ec, tcp::resolver::iterator _iter)
					{
						if (errorCheck<OP::Accept>(ec))
							return;

						// Successfully connected, do handshake
						m_webSocket.next_layer().async_handshake(ssl::stream_base::client,
							[this,cb = std::move(cb)](boost::system::error_code ec)
							{
								if (errorCheck<OP::SslHandshake>(ec))
									return;

								// Phew finally, call the callers cb
								cb();
							}
						);
					}
				);
			}
		);
	}

	void write(memory::Heap payload, WriteCallback cb) override
	{
		// Submit the write to the service and bootstrap the callbacks
		boost::asio::mutable_buffer buf(payload.cast<void *>(), payload.size());
		boost::asio::async_write(m_webSocket.next_layer(), buf,
			[this,payload = std::move(payload), cb = std::move(cb)](boost::system::error_code ec, size_t sizeWritten)
			{
				if (errorCheck<OP::Write>(ec))
					return;

				DCORE_ASSERT(sizeWritten == payload.size());
				cb();
			}
		);
	}

	// We lazily instantiate these as the class is used as a server or a resolving connector
	std::unique_ptr<tcp::acceptor> m_acceptor;
	std::unique_ptr<tcp::resolver> m_resolver;

	mutable tcp::socket m_socket;
	mutable websocket::stream<ssl::stream<tcp::socket&>> m_webSocket;
	SslContext m_sslContext;
};

}
