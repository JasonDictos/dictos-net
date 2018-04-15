#include <tests.hpp>
#include <catch.hpp>

using namespace dictos::net;
using namespace dictos;
using namespace dictos::async;
using namespace dictos::string::literals;
using namespace dictos::literals;

// @@ TODO
#if 0
TEST_CASE("Session::Basic")
{
	// Connect two sockets
	auto [clientSocket, serverSocket] = allocateStreamPair();

	// Bind them to a session
	auto client = std::make_shared<Session>(clientSocket);
	auto server = std::make_shared<Session>(serverSocket);

	// Send some commands
	client->submitCommand(Command("hello"));

	// Handle the recv assertion check
	server->recvCommand(
		[](Command request)
		{
			REQUIRE(request.getMethod() == "hello");
		}
	);

	// Listen to errors on the sessions
	std::atomic<bool> failed = false;
	auto c1 = server->ErrorSig.connect(
		[&failed](const dictos::error::Exception &e, net::OP op, StreamPtr stream)
		{
			net::GlobalContext().stop();
			failed = true;
		}
	);

	auto c2 = client->ErrorSig.connect(
		[&failed](const dictos::error::Exception &e, net::OP op, StreamPtr stream)
		{
			net::GlobalContext().stop();
			failed = true;
		}
	);

	// And give control to the implied global context to run
	net::GlobalContext().run();
	REQUIRE(failed == false);
}
#endif

// @@ TODO
#if 0
TEST_CASE("Session::ServerClient")
{
	Address addr("tcp://127.0.0.1:5120");
	Server<Session> server;

	// Ready our tcp server
	server.initialize(addr);

	// Listen when a new session is established
	auto c1 = server.ConnectSig.connect(
		[](SessionPtr &session)
		{
			// Got a session...
		}
	);

	// Now connect to this session server
	Session::connect(addr
		[](SessionPtr &session)
		{
			// Connected to the server
			session->submitRequest(Command("test_message",

			);
		}
	);

	// Listen to errors on the sessions
	std::atomic<bool> failed = false;
	auto c2 = server->ErrorSig.connect(
		[&failed](const dictos::error::Exception &e, net::OP op, StreamPtr stream)
		{
			net::GlobalContext().stop();
			failed = true;
		}
	);

	auto c3 = client->ErrorSig.connect(
		[&failed](const dictos::error::Exception &e, net::OP op, StreamPtr stream)
		{
			net::GlobalContext().stop();
			failed = true;
		}
	);

	net::GlobalContext().run();
	REQUIRE(failed == false);
}
#endif
