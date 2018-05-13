#pragma once

namespace dictos::net::protocol {

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

/**
 */
class Ssl : public AbstractProtocol
{
public:
	Ssl(Address addr, EventMachine &em, config::Context &config, ErrorCallback ecb, SslContext sslContext) :
		AbstractProtocol(std::move(addr), em, config, std::move(ecb)),
		m_socket(m_em, sslContext), m_sslContext(std::move(sslContext))
	{
		// @@ TODO
	}

	Ssl(Address addr, config::Context &config, ErrorCallback ecb, SslContext sslContext) :
		Ssl(std::move(addr), GlobalEventMachine(), config, std::move(ecb), std::move(sslContext))
	{
		// @@ TODO
	}

	void close() noexcept override
	{
		//m_socket.close(websocket::close_code::normal);
	}

	void accept(ProtocolUPtr &newProtocol, const AcceptCallback &cb) override
	{
		// @@ TODO
	}

	void read(Size size, const ReadCallback &cb) const override
	{
		// Allocate the buffer to read into for the caller
		memory::Heap result(size);

		// Construct an asio buffer before we move the result into the closure, due to
		// parameter initialization order this prevents a crash since result will
		// get moved before it gets passed into the async_read call.
		boost::asio::mutable_buffer buf(result.cast<void *>(), result.size());

		// Submit the read to the service and bootstrap the callbacks
		boost::asio::async_read(m_socket.next_layer(), buf,
			[this,result = std::move(result), &cb](boost::system::error_code ec, size_t sizeRead)
			{
				if (errorCheck<OP::Read>(ec))
					return;

				DCORE_ASSERT(sizeRead == result.size());
				cb(std::move(*const_cast<memory::Heap *>(&result)));
			}
		);
	}

	void connect(const ConnectCallback &cb) override
	{
		// First we go through a few hoops to resolve the address
		m_resolver = std::make_unique<tcp::resolver>(m_em);
		m_resolver->async_resolve(tcp::v4(), m_localAddress.ip(), string::toString(m_localAddress.port()),
			[this,&cb](boost::system::error_code ec, tcp::resolver::iterator results)
			{
				if (errorCheck<OP::Resolve>(ec))
					return;

				// Ok connect for each resolved entry, first one that connects ok will stop the enum
				boost::asio::async_connect(m_socket.next_layer(), results,
					[this,&cb](boost::system::error_code ec, tcp::resolver::iterator _iter)
					{
						if (errorCheck<OP::Accept>(ec))
							return;

						// Successfully connected, do handshake
						m_socket.async_handshake(ssl::stream_base::client,
							[this,&cb](boost::system::error_code ec)
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

	void write(memory::Heap payload, const WriteCallback &cb) override
	{
		// Submit the write to the service and bootstrap the callbacks
		boost::asio::mutable_buffer buf(payload.cast<void *>(), payload.size());
		boost::asio::async_write(m_socket.next_layer(), buf,
			[this,payload = std::move(payload), &cb](boost::system::error_code ec, size_t sizeWritten)
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

	mutable ssl::stream<tcp::socket> m_socket;

	SslContext m_sslContext;
};

}
