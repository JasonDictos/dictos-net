#pragma once

namespace dictos::net::protocol {

	enum class TYPE
	{
		Init,
		Tcp,
		Ssl,
		Udp,
		UnixDomain,
		File,
		Pipe,
		WebSocket,
		SslWebSocket
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
		case TYPE::Tcp:
			return stream << "Tcp";
		case TYPE::Ssl:
			return stream << "Ssl";
		case TYPE::UnixDomain:
			return stream << "UnixDomain";
		case TYPE::File:
			return stream << "File";
		case TYPE::Pipe:
			return stream << "Pipe";
		case TYPE::WebSocket:
			return stream << "WebSocket";
		case TYPE::SslWebSocket:
			return stream << "SslWebSocket";
		default:
			DCORE_ASSERT(!"Invalid protocol type ");
			return stream << "Invalid(" << static_cast<uint32_t>(type) << ")";
	}
}
