#include <tests.hpp>
#include <catch.hpp>

using namespace dictos::net;
using namespace dictos;

TEST_CASE("Address::Basic")
{
	Address address("tcp://127.0.0.1:555");
	REQUIRE(address.protocol() == PROTOCOL_TYPE::TCPv4);
	REQUIRE(address.port() == 555);
}
