#include <tests.hpp>
#include <catch.hpp>

using namespace dictos::net;
using namespace dictos;
using namespace dictos::async;
using namespace dictos::string::literals;
using namespace dictos::literals;

TEST_CASE("Stream::Basic")
{
	Address addr("tcp://127.0.0.1:5120");

	// Allocate a listening socket
	auto server = allocateStream(addr);

	memory::Heap writePayload(1_mb);
	writePayload.memset('A');

	// On each accepted stream read a 1MB payload and verify its size
	LOG(test, "Accepting test start");
	server->accept(
		[&writePayload](StreamPtr stream)
		{
			LOG(test, "Accept callback triggered for new stream:", *stream);

			stream->read(1_mb,
				[stream,&writePayload](memory::Heap payload)
				{
					LOG(test, "Verifying payload size:", payload.size());
					REQUIRE(payload.size() == 1_mb);
					REQUIRE(payload == writePayload);
				}
			);
		}
	);

	LOG(test, "Allocating client stream");

	// Now for the client just connect to ourselves and send a 1mb payload
	auto client = allocateStream(addr);

	LOG(test, "Successfully allocated client stream:", *client, "Connecting");

	LOG(test, "Sleep completed");

	client->connect(
		[client,&writePayload]()
		{
			LOG(test, "Connect succeeded writing");
			client->write(writePayload,
				[client]()
				{
					LOG(test, "Successfully wrote 1MB");
				}
			);
		}
	);

	// Listen to errors on the streams
	std::atomic<bool> failed = false;
	auto c1 = server->ErrorSig.connect(
		[&failed](const dictos::error::Exception &e, net::OP op, StreamPtr stream)
		{
			LOG(test, "Server - Error sig called:", e, '\n', e.traceString());
			net::GlobalContext().stop();
			failed = true;
		}
	);
	auto c2 = client->ErrorSig.connect(
		[&failed](const dictos::error::Exception &e, net::OP op, StreamPtr stream)
		{
			LOG(test, "Client - Error sig called:", e, '\n', e.traceString());
			net::GlobalContext().stop();
			failed = true;
		}
	);

	net::GlobalContext().run();
	REQUIRE(failed == false);
}
