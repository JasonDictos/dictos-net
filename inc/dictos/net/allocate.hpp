#pragma once

namespace dictos::net {

inline auto allocateStream(Address addr, EventMachine &em, config::Options options = config::Options())
{
	return std::make_shared<Stream>(std::move(addr), em, std::move(options));
}

inline auto allocateStream(Address addr, config::Options options = config::Options())
{
	return std::make_shared<Stream>(std::move(addr), std::move(options));
}

/**
 * Allocates a stream pair using a unix domain socket duped socket, most useful
 * for IPC.
 */
inline auto allocateStreamPair(config::Options options = config::Options())
{
	// @@ TODO
}

}
