#pragma once

namespace dictos::net::protocol {

using tcp = boost::asio::ip::tcp;

/**
 * The Tcp protocol implements an tcp v4/v6 protocol object for servicing
 * servers and client socket connections over ip systems.
 */
class Tcp : public AbstractProtocol
{
public:
	Tcp(Address addr, config::Context &config, ErrorCallback ecb) :
		AbstractProtocol(std::move(addr), config, std::move(ecb)),
		m_socket(GlobalContext())
	{
	}

	void close() override
	{
		m_socket.close();
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
			staticUPtrCast<Tcp>(newProtocol)->m_socket,
			[this,cb = std::move(cb)](boost::system::error_code ec) mutable
			{
				if (errorCheck<OP::Accept>(ec))
					return;

				// Call the caller back, its their job to associate this callback with the newly
				// connected protocol
				cb();
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
		boost::asio::async_read(m_socket, buf,
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
				boost::asio::async_connect(m_socket, results,
					[this,cb = std::move(cb)](boost::system::error_code ec, tcp::resolver::iterator _iter)
					{
						if (errorCheck<OP::Accept>(ec))
							return;

						// Successfully connected
						cb();
					}
				);
			}
		);
	}

	void write(memory::Heap payload, WriteCallback cb) override
	{
		// Submit the write to the service and bootstrap the callbacks
		boost::asio::mutable_buffer buf(payload.cast<void *>(), payload.size());
		boost::asio::async_write(m_socket, buf,
			[this,payload = std::move(payload), cb = std::move(cb)](boost::system::error_code ec, size_t sizeWritten)
			{
				if (errorCheck<OP::Write>(ec))
					return;

				DCORE_ASSERT(sizeWritten == payload.size());
				cb();
			}
		);
	}

	std::unique_ptr<tcp::acceptor> m_acceptor;
	std::unique_ptr<tcp::resolver> m_resolver;
	mutable tcp::socket m_socket;
};

}
