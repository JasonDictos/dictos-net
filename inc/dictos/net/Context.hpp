#pragma once

namespace dictos::net {

class Context
{
public:
	Context() = default;

	virtual ~Context()
	{
		LOGT(net, "Context shutting down");
		m_asioContext.stop();
	}

	void run()
	{
		LOGT(net, "Run called");
		m_asioContext.run();
	}

	void runOne()
	{
		LOGT(net, "Run one called");
		m_asioContext.run_one();
	}

	void stop()
	{
		LOGT(net, "Stop called");
		m_asioContext.stop();
	}

	operator boost::asio::io_context & () noexcept { return m_asioContext; }
	operator const boost::asio::io_context & () const noexcept { return m_asioContext; }

protected:
	boost::asio::io_context m_asioContext;
};

}
