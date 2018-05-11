#pragma once

namespace dictos::net {

/**
 * The net 'EventMachine' is where the threads get to participate in the asio state machine
 * loop. You can either call the apis on a context to process it with  your current thread
 * or you can create a context that has in it a pre-set number of dedicated threads.
 */
class EventMachine
{
public:
	EventMachine(uint32_t threadCount = 0)
	{
		for (uint32_t threadIdx = 0; threadIdx < threadCount; threadIdx++) {
			m_threads.push_back(std::make_unique<async::Thread>(
				string::toString("Context service thread:", threadIdx),
				std::bind(&EventMachine::run, this)
			));
		}
	}

	virtual ~EventMachine()
	{
		for(auto &thread : m_threads) {
			thread->cancel();
		}
		m_asioContext.stop();
		m_threads.clear();
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
	std::vector<std::unique_ptr<async::Thread>> m_threads;
};

}