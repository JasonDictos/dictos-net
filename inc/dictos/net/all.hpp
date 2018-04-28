namespace dictos {
}

// Bind core's namespace into ours
namespace dictos::net {
	using namespace ::dictos;
}

#include <dictos/net/external.hpp>
#include <dictos/core/all.hpp>
#include <dictos/net/json.hpp>
#include <dictos/net/Command.hpp>
#include <dictos/net/Context.hpp>
#include <dictos/net/api.hpp>
#include <dictos/net/streamptr.hpp>
#include <dictos/net/error/all.hpp>
#include <dictos/net/op.hpp>
#include <dictos/net/protocol/all.hpp>

// Stick in the type enum in our parent namespace
namespace dictos::net {
	using PROTOCOL_TYPE = protocol::TYPE;
}

#include <dictos/net/Address.h>
#include <dictos/net/protocol/all2.hpp>
#include <dictos/net/Address.hpp>
#include <dictos/net/Stream.hpp>
#include <dictos/net/allocate.hpp>
