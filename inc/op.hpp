#pragma once

namespace dictos::net {

	enum class OP
	{
		Connect,
		Accept,
		Read,
		Write,
		Resolve,
	};

}

// Allow OP to be renderable in our log/error apis
inline std::ostream & operator << (std::ostream &stream, ::dictos::net::OP operation)
{
	switch (operation)
	{
		case ::dictos::net::OP::Connect:
			return stream << "Connect";
		case ::dictos::net::OP::Accept:
			return stream << "Accept";
		case ::dictos::net::OP::Read:
			return stream << "Read";
		case ::dictos::net::OP::Write:
			return stream << "Write";
		case ::dictos::net::OP::Resolve:
			return stream << "Resolve";
		default:
			DCORE_ASSERT(!"Invalid operation code");
			return stream << "Invalid(" << static_cast<uint32_t>(operation) << ")";
	}
}