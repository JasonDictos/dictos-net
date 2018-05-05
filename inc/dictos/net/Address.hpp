#pragma once

namespace dictos::net {

inline Address::Address(const Address &addr)
{
	operator = (addr);
}

inline Address::Address(Address &&addr)
{
	operator = (std::move(addr));
}

inline Address &Address::operator = (const Address &addr)
{
	if (this == &addr)
		return *this;

	m_protocol = addr.m_protocol;
	m_address = addr.m_address;
	m_port = addr.m_port;
	return *this;
}

inline Address &Address::operator = (Address &&addr)
{
	m_protocol = addr.m_protocol;
	m_address = std::move(addr.m_address);
	m_port = addr.m_port;
	addr.m_protocol = PROTOCOL_TYPE::Init;
	addr.m_port = 0;
	addr.m_address = IpAddress();
	return *this;
}

inline Address::Address(const std::string &addr)
{
	operator = (addr);
}

inline Address & Address::operator = (const std::string &addr)
{
	m_address = validate(addr, m_port, m_protocol);
	return *this;
}

inline unsigned short Address::port() const
{
	if (m_protocol == PROTOCOL_TYPE::Pipe || m_protocol == PROTOCOL_TYPE::UnixDomain)
		DCORE_THROW(RuntimeError, "Address type does not support a port number");
	return m_port;
}

inline std::string Address::ip() const
{
	if (m_protocol != PROTOCOL_TYPE::Tcp && m_protocol != PROTOCOL_TYPE::Tcp)
		DCORE_THROW(RuntimeError, "Address type does not support an ip");
	return m_address.to_string();
}

inline std::string Address::__toString() const
{
	if (m_port)
		return string::toString(m_protocol, "://", m_address, ":", m_port);
	return string::toString(m_protocol, "://", m_address);
}

inline PROTOCOL_TYPE Address::protocol() const
{
	return m_protocol;
}

inline Address::operator bool () const
{
	return m_protocol != PROTOCOL_TYPE::Init;
}

inline Address::IpAddress Address::validate(const std::string &__address, unsigned short &_port, TYPE &type)
{
	// Extract the protocol type and look it up in the protocol registrar
	auto [prefix, _address] = string::split("://", __address);
	type = protocol::lookup(prefix);

	// Extract the port
	auto [address, port] = string::split(":", _address);

	// Now we can have boost parse the ip portion
	boost::system::error_code ec;
	auto addr = IpAddress::from_string(address, ec);
	if (ec)
		DCORE_THROW(InvalidArgument, "Un-recognized address:", address, ec);

	// If tcp fetch port
	switch (type) {
		case PROTOCOL_TYPE::Tcp:
		case PROTOCOL_TYPE::Udp:
		case PROTOCOL_TYPE::WebSocket:
		case PROTOCOL_TYPE::SslWebSocket:
			if (!port.empty())
				_port = string::toNumber<unsigned short>(port);
		break;
	}

	// And return the ip address object
	return addr;
}

}
