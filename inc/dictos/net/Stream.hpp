#pragma once

namespace dictos::net {

/**
 * The stream object represents a connected endpoint for data i/o transmission. It encapsulated
 * the concept of a protocol object which is selected based on the address prefix.
 */
class Stream :
	public config::Context,
	public std::enable_shared_from_this<Stream>
{
public:
	~Stream()
	{
		LOGT(stream, "Deconstructing");
		close();
	}

	// Define the well known op callback signatures
	typedef std::function<void(StreamPtr)> AcceptCallback;
	using ReadCallback = protocol::AbstractProtocol::ReadCallback;
	using ConnectCallback = protocol::AbstractProtocol::ConnectCallback;
	using WriteCallback = protocol::AbstractProtocol::WriteCallback;
	using ErrorCallback = protocol::AbstractProtocol::ErrorCallback;

	Stream(Address addr, EventMachine &em, config::Options options = config::Options()) :
		Context(getSection(), std::move(options)),
		m_protocol(protocol::allocateProtocol(std::move(addr), *this,
			std::bind(&Stream::onError, this, std::placeholders::_1, std::placeholders::_2), em))
	{
	}

	Stream(Address addr, config::Options options = config::Options()) :
		Stream(std::move(addr), GlobalEventMachine(), std::move(options))
	{
	}

	std::string __toString() const
	{
		return string::toString("Stream(", getLocalAddress(), ")");
	}

	StreamPtr getStreamPtr() const
	{
		return const_cast<Stream *>(this)->enable_shared_from_this<Stream>::shared_from_this();
	}

	void accept(AcceptCallback cb)
	{
		try {
			LOGT(stream, "Accepting new connections");

			// Create a new blank stream from our target address and options
			auto newStream = std::make_shared<Stream>(getLocalAddress(), getOptions());
			auto &protocol = newStream->m_protocol;

			// Now transfer that stream through the callback, and hand the accept call the protocol
			// ptr so it can set it up for us
			m_protocol->accept(
				protocol,
				[this,bookmark = getStreamPtr(),cb = std::move(cb),newStream = std::move(newStream)]() {
					// Great we successfully accepted a new connection
					LOGT(stream, "Successfully accepted new connection from:", newStream->getRemoteAddress());
					cb(std::move(newStream));
				}
			);
		} catch (dictos::error::Exception &e) {
			LOG(stream, __FUNCTION__, "error re-raise", e);
			throw;
		} catch (std::exception &e) {
			DCORE_ERR_THROW(net::error::NetException, "Failed to accept:", e);
		}
	}

	void read(Size size, ReadCallback cb) const
	{
		try {
			LOGT(stream, "Reading:", size);

			m_protocol->read(size, 
				[this,stream = getThisPtr(),cb = std::move(cb)](memory::HeapView data) {

				// Now that we've received it report our rate
				RecvRate.report(data.size());

				cb(std::move(data));
			});
		} catch (dictos::error::Exception &e) {
			LOG(stream, __FUNCTION__, "error re-raise", e);
			throw;
		} catch (std::exception &e) {
			DCORE_ERR_THROW(net::error::NetException, "Failed to read:", e);
		}
	}

	void connect(ConnectCallback cb)
	{
		try {
			LOGT(stream, "Connecting");

			m_protocol->connect(std::move(cb));
		} catch (dictos::error::Exception &e) {
			LOG(stream, __FUNCTION__, "error re-raise", e);
			throw;
		} catch (std::exception &e) {
			DCORE_ERR_THROW(net::error::NetException, "Failed to connect:", e);
		}
	}

	void write(memory::Heap payload, WriteCallback cb = WriteCallback())
	{
		try {
			LOGT(stream, "Writing:", payload.size());

			auto size = payload.size();

			m_protocol->write(std::move(payload), 
				[this,size,stream = getThisPtr(),cb = std::move(cb)]() {

					// Now that we've sent it report it to stats
					SendRate.report(size);

					// Write callbacks are optional
					if (cb) {
						cb();
					}
				}
			);
		} catch (dictos::error::Exception &e) {
			LOG(stream, __FUNCTION__, "error re-raise", e);
			throw;
		} catch (std::exception &e) {
			DCORE_ERR_THROW(net::error::NetException, "Failed to write:", e);
		}
	}

	void close()
	{
		try {
			LOGT(stream, "Closing");

			m_protocol->close();
		} catch (dictos::error::Exception &e) {
			LOG(stream, __FUNCTION__, "error re-raise", e);
			throw;
		} catch (std::exception &e) {
			DCORE_ERR_THROW(net::error::NetException, "Failed to close:", e.what());
		}
	}

	Address getLocalAddress() const
	{
		try {
			return m_protocol->getLocalAddress();
		} catch (dictos::error::Exception &e) {
			LOG(stream, __FUNCTION__, "error re-raise", e);
			throw;
		} catch (std::exception &e) {
			DCORE_ERR_THROW(net::error::NetException, "Failed to get local address:", e);
		}
	}

	Address getRemoteAddress() const
	{
		try {
			return m_protocol->getRemoteAddress();
		} catch (dictos::error::Exception &e) {
			LOG(stream, __FUNCTION__, "error re-raise", e);
			throw;
		} catch (std::exception &e) {
			DCORE_ERR_THROW(net::error::NetException, "Failed to get remote address:", e);
		}
	}

	/**
	 * Will re-throw any previously caught exception.
	 */
	void checkLastError() const
	{
		auto guard = m_lock.lock();
		if (m_lastError)
			std::rethrow_exception(m_lastError);
	}

	EventMachine &eventMachine() { return m_protocol->eventMachine(); }

	// Error handling is centralized to this public signal for
	// clients to handle errors centrally as well
	signals::signal<
		void(const dictos::error::Exception &e, OP op, StreamPtr)
		> ErrorSig;

	mutable util::Throughput SendRate;
	mutable util::Throughput RecvRate;

protected:
	StreamPtr getThisPtr() const
	{
		return const_cast<Stream *>(this)->enable_shared_from_this<Stream>::shared_from_this();
	}

	/**
	 * Called by the protocol on an error.
	 */
	void onError(const dictos::error::Exception &e, OP operation)
	{
		auto guard = m_lock.lock();
		m_lastError = std::make_exception_ptr(e);
		guard.release();

		// Pass it along and grab a strong ref to ourselves along the way
		LOGT(net, "Protocol reported error:", e, "For operation:", operation);
		ErrorSig(e, operation, shared_from_this());
	}

	static const config::Section & getSection()
	{
		if (auto section = config::Sections::find("net_stream"))
			return *section;

		static config::Section section("net_stream", {
				{"private_key_path", file::path(), "Path to client private ke (for client based auth)y"},
				{"client_cert_file", file::path(), "Path to client cert file key (for client based auth)"},
				{"cert_chain_file", file::path(), "Path to cert chain file (for peer certificate validation)"},
				{"verify_peer", true, "Whether to verify the peer" }
			}
		);

		return section;
	}

	mutable async::MutexLock m_lock;
	std::exception_ptr m_lastError;

	protocol::ProtocolUPtr m_protocol;
};

}
