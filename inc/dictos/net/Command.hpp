#pragma once

namespace dictos::net {

/**
 * A partway basic json rpc clone representing a json method with params. Although
 * it uses json as its core translation type, other serialization targets may be
 * possible, hence the name of this class is kept generic, however json provides
 * basic types most easily represented across every major language out there so
 * its a good type system for any rpc serialization.
 */
class Command
{
public:
	Command() = default;

	Command(std::string_view method, json params) :
		m_method(method.begin(), method.end()),
		m_params(std::move(params))
	{
	}

	void setResult(json result)
	{
		m_result = std::move(result);
	}

	void setError(json error)
	{
		m_error = std::move(error);
	}

	Command(const Command &cmd)
	{
		operator = (cmd);
	}

	Command(Command &&cmd)
	{
		operator = (std::move(cmd));
	}

	Command & operator = (const Command &cmd)
	{
		if (this == &cmd)
			return *this;

		m_method = cmd.m_method;
		m_params = cmd.m_params;
		m_id = cmd.m_id;
		return *this;
	}

	Command & operator = (Command &&cmd)
	{
		m_method = std::move(cmd.m_method);
		m_params = std::move(cmd.m_params);
		m_id = std::move(cmd.m_id);
		return *this;
	}

	const std::string &method() const { return m_method; }
	std::string method() { return m_method; }

	const json &params() const { return m_params; }
	json &params() { return m_params; }

	const json &result() const { return m_result; }
	json &result() { return m_result; }

	const json &error() const { return m_error; }
	json &error() { return m_error; }

	const Uuid &id() const { return m_id; }
	Uuid &id() { return m_id; }

	enum class TYPE {
		Init,
		Request,
		Result,
		Error
	};

	TYPE type() const
	{
		if (!m_error.empty())
			return TYPE::Error;
		else if (!m_result.empty())
			return TYPE::Result;
		else if (!m_params.empty() && !m_method.empty())
			return TYPE::Request;
		else
			return TYPE::Init;
	}

	bool operator < (const Command &command) const
	{
		return m_id < command.m_id;
	}

protected:
	std::string m_method;
	json m_params, m_result, m_error;
	Uuid m_id;
	const std::string m_jsonRpcVersion = "2.0";
};

}

namespace ns {

using nlohmann::json;
using namespace dictos;

// Conversion hooks for Command to json/from json
inline void to_json(json& j, const net::Command& cmd)
{
	switch (cmd.type()) {
		case net::Command::TYPE::Request:
			j = json{{"jsonrpc", "2.0"}, {"id", cmd.id()}, {"method", cmd.method()}, {"params", cmd.params()}};
			break;
		case net::Command::TYPE::Result:
			j = json{{"jsonrpc", "2.0"}, {"id", cmd.id()}, {"result", cmd.result()}};
			break;
		case net::Command::TYPE::Error:
			j = json{{"jsonrpc", "2.0"}, {"id", cmd.id()}, {"error", cmd.result()}};
			break;
		default:
			DCORE_THROW(RuntimeError, "Cannot convert an un-setup command to json");
	}
}

inline void from_json(const json& j, net::Command& cmd)
{
	if (j.find("method") != j.end()) {
		cmd.params() = j.at("params").get<json>();
		cmd.method() = j.at("method").get<std::string>(),
		cmd.id() = Uuid::__fromString(j.at("id").get<std::string>());
	} else if (j.find("result") != j.end()) {
		cmd.result() = j.at("params").get<json>();
		cmd.id() = Uuid::__fromString(j.at("id").get<std::string>());
	} else if (j.find("error") != j.end()) {
		cmd.error() = j.at("error").get<json>();
		cmd.id() = Uuid::__fromString(j.at("id").get<std::string>());
	} else {
		DCORE_THROW(RuntimeError, "Invalid json payload");
	}
}

}
