#pragma once

namespace dictos::net {

/**
 * An address represents an endpoint to connect to, usually
 * includes the port, can be an ip address, domain name,
 * or a file path (unix domain). Custom values can be
 * defined by registering a new protocol prefix then
 * implementing a new protocol object derived from 
 * protocol::AbstractProtocol.
 */
class Address
{
public:
	using IpAddress = boost::asio::ip::address;

	Address() = default;

	Address(const Address &addr);
	Address(Address &&addr);
	Address(const std::string &addr);

	Address &operator = (const Address &addr);
	Address &operator = (Address &&addr);
	Address &operator = (const std::string &addr);

public:
	unsigned short port() const;

	std::string ip() const;

	std::string __toString() const;
	PROTOCOL_TYPE protocol() const;
	explicit operator bool () const;

protected:
	static IpAddress validate(const std::string &_address, unsigned short &port, PROTOCOL_TYPE &type);

	unsigned short m_port = 0;
	IpAddress m_address;
	PROTOCOL_TYPE m_protocol = PROTOCOL_TYPE::Init;
};

}
