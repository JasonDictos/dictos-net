#pragma once

namespace dictos::net::protocol {

/**
 * The abstract protocol defines the interface and common logic for implementing
 * a protocol object to be used in a stream. Note: In regards to lifetime management
 * with boost asio async callbacks, the protocols themselves do not have to worry about it
 * as the stream container object will itself capture its own shared ptr when submitting
 * callback operations. This simplifies the protocols themselvs and allows us to share that
 * logic across all protocols.
 */
class AbstractProtocol
{
public:
	// Define the well known op callback signatures, they are slightly different then
	// what the stream wrapper exposes
	typedef std::function<void()> AcceptCallback;
	typedef std::function<void(memory::Heap)> ReadCallback;
	typedef std::function<void()> ConnectCallback;
	typedef std::function<void()> WriteCallback;
	typedef std::function<void(const dictos::error::Exception &, OP)> ErrorCallback;

	AbstractProtocol(Address addr, config::Context &config, ErrorCallback ecb) :
		m_config(config), m_localAddress(std::move(addr)), m_ecb(std::move(ecb)), m_service(GlobalContext())
	{
	}

	virtual ~AbstractProtocol() = default;

	// Server accept/listen/bind
	virtual void accept(ProtocolUPtr &newProtocol, AcceptCallback cb) = 0;

	// Client connect
	virtual void connect(ConnectCallback cb) = 0;

	// Close
	virtual void close() = 0;

	// I/O
	virtual void read(Size size, ReadCallback cb) const = 0;
	virtual void write(memory::Heap payload, WriteCallback cb) = 0;

	Address getLocalAddress() const { return m_localAddress; }
	Address getRemoteAddress() const { return m_localAddress; }

protected:
	/**
	 * Returns true if ec was set, and along the way sends the exception
	 * through our error signal.
	 */
	template<OP OpType, class ExceptionType = net::error::NetException>
	bool errorCheck(boost::system::error_code ec, const std::string_view &message = std::string_view()) const
	{
		if (ec && ec.value() != boost::system::errc::operation_canceled)
		{
			// Pipe all errors through the error handler
			m_ecb(
				DCORE_ERR_MAKE(ExceptionType, message, ec.message()),
				OpType
			);
			return true;
		}
		return false;
	}

	template<class Type>
	Type getOption(const std::string_view &key) const { return m_config.getOption<Type>(key); }

	// We'll use the callers config context for general net options
	config::Context &m_config;

	std::atomic<bool> m_connect = {false};
	std::atomic<bool> m_accept = {false};

	ErrorCallback m_ecb;

	boost::asio::io_service &m_service;
	Address m_localAddress, m_remoteAddress;
};

}
