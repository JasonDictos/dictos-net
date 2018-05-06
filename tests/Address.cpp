#include <tests.hpp>
#include <catch.hpp>

using namespace dictos::net;
using namespace dictos;

TEST_CASE("Address::Basic")
{
	Address address("tcp://127.0.0.1:555");
	REQUIRE(address.protocol() == PROTOCOL_TYPE::Tcp);
	REQUIRE(address.port() == 555);

	address = Address("ws://127.0.0.1:555");
	REQUIRE(address.protocol() == PROTOCOL_TYPE::WebSocket);
	REQUIRE(address.port() == 555);

	address = Address("wss://127.0.0.1:555");
	REQUIRE(address.protocol() == PROTOCOL_TYPE::SslWebSocket);
	REQUIRE(address.port() == 555);
}
