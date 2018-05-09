#define CATCH_CONFIG_RUNNER
#include <tests.hpp>
#include <catch.hpp>
#include <dictos/core/test/CatchListener.hpp>

using namespace dictos;

namespace {
	static log::StreamBuffer s_stream("Catch");
	static std::streambuf * s_oldBuffer = nullptr;
}

// Register our listener for logging
CATCH_REGISTER_LISTENER(CatchListener);

int main (int argc, char * argv[])
{
	// Redirect std::cout rdbuf to go to our logger
	s_oldBuffer = std::cout.rdbuf(&s_stream);

	// Enable testing output
	log::enableLevel("all");

	// Do the catch thing
	Catch::Session().run( argc, argv );

	// Stop our global service so we don't crash
	net::GlobalEventMachine().stop();
}
