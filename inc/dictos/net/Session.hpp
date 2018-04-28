#pragma once

namespace dictos::net {

/**
 * The session is a higher level connection that handles
 * the encapsulation of Command objects across a network to
 * a peer session.
 */
class Session
{
public:
	Session() = default;

	/**
	 * Submits the payload to the stream for async sending.
	 */
	template<class Payload>
	void send(Payload payload)
	{
		m_stream->write(std::move(payload));
	}

	// Error handling is centralized to this public signal for
	// clients to handle errors centrally as well
	boost::signals2::signal<
		void(const dictos::error::Exception &e, OP op, StreamPtr)
		> ErrorSig;

protected:
	StreamPtr m_stream;
};

}
