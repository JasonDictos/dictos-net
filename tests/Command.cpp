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

	REQUIRE(request.method() == "hello");

	request.params()["param1"] = 1.5;

	REQUIRE(request.params()["param1"] == 1.5);

	json j = request;

	Command request2 = j;

	REQUIRE(request2.method() == "hello");
	REQUIRE(request2.params()["param1"] == 1.5);
}
