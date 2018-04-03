#pragma once

namespace dictos::net::protocol {

	enum class TYPE
	{
		Init,
		TCPv4,
		TCPv6,
		UDP,
		Unix,
		File,
		Pipe
	};
}

// Allow TYPE to be renderable in our log/error apis
inline std::ostream & operator << (std::ostream &stream, ::dictos::net::protocol::TYPE type)
{
	using TYPE = ::dictos::net::protocol::TYPE;

	switch (type)
	{
		case TYPE::Init:
			return stream << "Init";
		case TYPE::TCPv4:
			return stream << "TCPv4";
		case TYPE::TCPv6:
			return stream << "TCPv6";
		case TYPE::UDP:
			return stream << "UDP";
		case TYPE::Unix:
			return stream << "Unix";
		case TYPE::File:
			return stream << "File";
		case TYPE::Pipe:
			return stream << "Pipe";
		default:
			DCORE_ASSERT(!"Invalid protocol type ");
			return stream << "Invalid(" << static_cast<uint32_t>(type) << ")";
	}
}
