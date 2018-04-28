#pragma once

namespace dictos::net::error {

/**
 * A net exception is a general net exception base class.
 */
class NetException : public ::dictos::error::Exception
{
public:
	using Exception::Exception;
};

}
