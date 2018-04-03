#pragma once

namespace dictos::net {

inline auto allocateStream(Address addr, config::Options options = config::Options())
{
	return std::make_shared<Stream>(std::move(addr), std::move(options));
}

}
