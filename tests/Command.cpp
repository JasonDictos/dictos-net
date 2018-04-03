#include <tests.hpp>
#include <catch.hpp>

using namespace dictos::net;
using namespace dictos;
using namespace dictos::async;
using namespace dictos::string::literals;
using namespace dictos::literals;

TEST_CASE("Command::Basic")
{
	Command request("hello");

	REQUIRE(request.getMethod() == "hello");

	request["param1"] = 1.5;

	REQUIRE(request["param1"] == 1.5);

	json j = request;

	Command request2 = j;

	REQUIRE(request2.getMethod() == "hello");
	REQUIRE(request2["param1"] == 1.5);
}
