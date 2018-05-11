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
	SslWebSocket(Address addr, EventMachine &em, config::Context &config, ErrorCallback ecb, SslContext sslContext) :
		AbstractProtocol(std::move(addr), em, config, std::move(ecb)),
		m_socket(m_em),
		m_sslContext(std::move(sslContext))
	{
		m_webSocket = std::make_unique<websocket::stream<ssl::stream<tcp::socket&>>>(m_socket, m_sslContext);
		m_webSocket->next_layer().set_verify_callback(boost::bind(&SslWebSocket::onVeirfyCertificate, this, _1, _2));
	}

	SslWebSocket(Address addr, config::Context &config, ErrorCallback ecb, SslContext sslContext) :
		SslWebSocket(std::move(addr), GlobalEventMachine(), config, std::move(ecb), std::move(sslContext))
	{
	}

	bool onVeirfyCertificate(bool preverified, boost::asio::ssl::verify_context &ctx)
	{
		return true;
	}

	void close() noexcept override
	{
		dictos::error::block([this]{ m_webSocket->close(websocket::close_code::normal); });
	}

	void accept(ProtocolUPtr &newProtocol, AcceptCallback cb) override
	{
		m_acceptor = std::make_unique<tcp::acceptor>(
			m_em,
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
			[this,cb = std::move(cb),&newProtocol](boost::system::error_code ec) mutable {
				if (errorCheck<OP::Accept>(ec))
					return;

				// Successfully connected, do handshake
				staticUPtrCast<SslWebSocket>(newProtocol)->m_webSocket->next_layer().async_handshake(ssl::stream_base::server,
					[this,cb = std::move(cb)](boost::system::error_code ec) {
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
		auto buffer = std::make_shared<boost::beast::multi_buffer>();
		auto &_buffer = *buffer;

		// Submit the read to the service and bootstrap the callbacks
		m_webSocket->async_read(
			_buffer,
			[this,buffer = std::move(buffer),cb = std::move(cb)](boost::system::error_code ec, size_t sizeRead) {
				boost::ignore_unused(sizeRead);

				if (errorCheck<OP::Read>(ec))
					return;

				auto data = buffer->data();
				auto begin = boost::asio::buffers_begin(data);
				auto end = boost::asio::buffers_end(data);
				cb(memory::Heap(begin, end));
			}
		);
	}

	void connect(ConnectCallback cb) override
	{
		// First we go through a few hoops to resolve the address
		m_resolver = std::make_unique<tcp::resolver>(m_em);
		m_resolver->async_resolve(tcp::v4(), m_localAddress.ip(), string::toString(m_localAddress.port()),
			[this,cb = std::move(cb)](boost::system::error_code ec, tcp::resolver::iterator results) {
				if (errorCheck<OP::Resolve>(ec))
					return;

				// Ok connect for each resolved entry, first one that connects ok will stop the enum
				boost::asio::async_connect(m_webSocket->next_layer().next_layer(), results,
					[this,cb = std::move(cb)](boost::system::error_code ec, tcp::resolver::iterator _iter) {
						if (errorCheck<OP::Accept>(ec))
							return;

						// Successfully connected, do ssl handshake
						m_webSocket->next_layer().async_handshake(ssl::stream_base::client,
							[this,cb = std::move(cb)](boost::system::error_code ec) {
								if (errorCheck<OP::SslHandshake>(ec))
									return;

								// One more handshake, the websocket one
								m_webSocket->handshake(m_localAddress.ip(), "/");
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
		m_webSocket->async_write(buf,
			[this,payload = std::move(payload), cb = std::move(cb)](boost::system::error_code ec, size_t sizeWritten) {
				if (errorCheck<OP::Write>(ec))
					return;

				DCORE_ASSERT(sizeWritten == payload.size());
				if (cb) cb();
			}
		);
	}

	// We lazily instantiate these as the class is used as a server or a resolving connector
	std::unique_ptr<tcp::acceptor> m_acceptor;
	std::unique_ptr<tcp::resolver> m_resolver;

	mutable boost::beast::multi_buffer m_buffer;

	mutable tcp::socket m_socket;
	mutable std::unique_ptr<websocket::stream<ssl::stream<tcp::socket&>>> m_webSocket;
	SslContext m_sslContext;
};

}
