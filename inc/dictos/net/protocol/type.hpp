#pragma once

namespace dictos::net::protocol {

	enum class TYPE {
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
inline std::ostream & operator << (std::ostream &stream, ::dictos::net::protocol::TYPE type) {
	using TYPE = ::dictos::net::protocol::TYPE;

	switch (type) {
		case TYPE::Init:
			return stream << "init";
		case TYPE::Tcp:
			return stream << "tcp";
		case TYPE::Ssl:
			return stream << "ssl";
		case TYPE::UnixDomain:
			return stream << "unix";
		case TYPE::File:
			return stream << "file";
		case TYPE::Pipe:
			return stream << "pipe";
		case TYPE::WebSocket:
			return stream << "ws";
		case TYPE::SslWebSocket:
			return stream << "wss";
		default:
			DCORE_THROW(RuntimeError, "Invalid protocol type:",  static_cast<uint32_t>(type));
	}
}
