#pragma once

namespace dictos::net {

/**
 * The session is a higher level connection that handles
 * the encapsulation of Command objects across a network to
 * a peer session.
 */
class Session :
	public util::SharedFromThis<Session>
{
public:
	typedef std::function<void(Command result)> ReplyHandler;

	Session(StreamPtr stream) :
		m_stream(std::move(stream))
	{
		// Link to the streams error signal
		m_errCon = m_stream->ErrorSig.connect([this](
			const dictos::error::Exception &e, OP op, StreamPtr stream)
		{ ErrorSig(e, thisPtr()); });
	}

	void close() {
		m_stream->close();
	}

	/**
	 * The request context remains around for the life of an outstanding
	 * or incoming request. It tracks the callback which will be triggered
	 * upon the completion of its registration in the queues, when we
	 * receive the associated reply.
	 */
	struct RequestCtx {
		Command request;
		ReplyHandler replyHandler;
	};

	/**
	 * Submits the payload to the stream for async sending.
	 */
	void submitRequest(Command cmd, std::optional<ReplyHandler> replyHandler = {}) {
		// Has to be a request
		if (cmd.type() != Command::TYPE::Request) {
			DCORE_THROW(RuntimeError, "Invalid command type:", cmd);
		}

		// Id can't be nil
		if (cmd.id() == Uuid::nill()) {
			DCORE_THROW(RuntimeError, "Id must not be nil for request:", cmd);
		}

		auto guard = m_lock.lock();

		// Ensure a duplicate id wasn't used
		auto iter = m_outgoing.find(cmd.id());

		if (iter != m_outgoing.end()) {
			DCORE_THROW(RuntimeError, "Duplicate outgoing request id detected:", cmd);
		}

		guard.release();

		// Add a request context for this request id
		auto id = cmd.id();
		auto json = string::toString(cmd);

		// May be unset, which implies no reply
		if (replyHandler) {
			LOGT(SESSION, "Registering a command context with id:", id);
			guard.lock();
			m_outgoing[id] = RequestCtx(
						{std::move(cmd), std::move(replyHandler.value())
					});
			guard.unlock();
		}

		// Submit it over the wire
		LOGT(SESSION, "Sending request:", json);
		m_stream->write(std::move(json), [this, &cmd]() {
			WriteSig(thisPtr(), cmd);
		});

		// And enqueue a read
		enqueueRead();
	}

	// Error handling is centralized to this public signal for
	// clients to handle errors centrally as well
	signals::signal<
		void(const dictos::error::Exception &e, SessionPtr)
		> ErrorSig;

	// This signal is emitted when we receive an incoming request,
	// gives the user a chance to handle it.
	signals::signal<
		void (SessionPtr session, const Command &request)
		> IncomingSig;

	// This signal gets raised every time we successfully complete a write.
	signals::signal<
		void (SessionPtr session, const Command &request)
		> WriteSig;

	StreamPtr stream() const { return m_stream; }

protected:
	/**
	 * Called when we receive an incoming payload
	 */
	void onIncoming(memory::HeapView data)
	{
		// Parse the command and figure out what to do with it
		Command cmd(std::move(data));

		switch (cmd.type()) {
			case Command::TYPE::Request:
				// Register it and notify the callback
				onIncomingRequest(std::move(cmd));
				break;

			case Command::TYPE::Result:
			case Command::TYPE::Error:
				onIncomingResult(std::move(cmd));
				break;

			default:
				DCORE_THROW(RuntimeError, "Invalid command received:", cmd);
		}

		// Queue up another read
		enqueueRead();
	}

	void enqueueRead()
	{
		m_stream->read(Size(), [session = getThisPtr()](memory::HeapView data) {
			session->onIncoming(std::move(data));
		});
	}

	/**
	 * This is called when we get a request command from the peer. We dispatch
	 * it to the callers registered incoming signal handler after registering
	 * it with a command context in the incoming map.
	 */
	void onIncomingRequest(Command request)
	{
		// Add it to our incoming queue then trigger the callers incoming handler
		auto guard = m_lock.lock();

		auto iter = m_incoming.find(request.id());

		if (iter != m_incoming.end()) {
			LOG(ERROR, "Received redundant request with duplicate id:", request, iter->second);
			return;
		}

		guard.unlock();

		IncomingSig(thisPtr(), std::move(request));
	}

	/**
	 * This is called when we get a result or error type. It implies we have
	 * an outstanding request in the outgoing queue which we will notify
	 * the stashed callback of the completed request.
	 */
	void onIncomingResult(Command result)
	{
		LOG(SESSION, "Received incoming result:", result);

		// We should have something in the outgoing queue matching its id
		auto guard = m_lock.lock();
		auto iter = m_outgoing.find(result.id());
		if (iter == m_outgoing.end()) {
			LOG(ERROR, "Ignoring incoming result for invalid id:", result);
			return;
		}

		// Ok move it out of there then dispatch the callback
		auto context = std::move(iter->second);
		m_outgoing.erase(iter);
		guard.release();

		try {
			context.replyHandler(std::move(result));
		} catch (dictos::error::Exception &e) {
			LOG(ERROR, "Result handler threw:", e);
			dictos::error::block([&](){ ErrorSig(e, thisPtr()); });
		}
	}

	void onStreamError(const dictos::error::Exception &e, OP op, StreamPtr stream) {
		ErrorSig(e, thisPtr());
	}

	signals::scoped_connection m_errCon;
	StreamPtr m_stream;
	async::SpinLock m_lock;
	std::map<Uuid, RequestCtx> m_outgoing, m_incoming;
};

}
