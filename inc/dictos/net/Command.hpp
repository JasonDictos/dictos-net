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
	Command() : m_id(Uuid::create())		// Create a new uuid implicitly upon blank instantiation
	{
	}

	Command(memory::HeapView data) :
		Command(json::parse(data.begin(), data.end()))
	{
	}

	Command(json j)
	{
		auto _param = j.find("params");
		auto _error = j.find("error");
		auto _method = j.find("method");
		auto _result = j.find("result");
		auto _id = j.find("id");

		if (_param != j.end())
			m_params = std::move(*_param);
		if (_error != j.end())
			m_error = std::move(*_error);
		if (_method != j.end())
			m_method = std::move(_method->get<std::string>());
		if (_id != j.end())
			m_id = Uuid::__fromString(_id->get<std::string>());	 // @@ TODO figure out the custom json overloads
		if (_result != j.end())
			m_result = std::move(*_result);
	}

	Command(std::string_view method, json params) :
		m_method(method.begin(), method.end()),
		m_params(std::move(params))
	{
	}

	Command(const Command &) = delete;
	Command & operator = (const Command &) = delete;

	Command(Command &&cmd) :
		m_id(std::move(cmd.m_id)),					// we don't re-use the operator = here since we want to avoid Uuid self generating an id
		m_params(std::move(cmd.m_params)),
		m_result(std::move(cmd.m_result)),
		m_error(std::move(cmd.m_error)),
		m_jsonRpcVersion(std::move(cmd.m_jsonRpcVersion)),
		m_method(std::move(cmd.m_method))
	{
	}

	Command & operator = (Command &&cmd)
	{
		m_id = std::move(cmd.m_id);
		m_params = std::move(cmd.m_params);
		m_result = std::move(cmd.m_result);
		m_error = std::move(cmd.m_error);
		m_jsonRpcVersion = std::move(cmd.m_jsonRpcVersion);
		m_method = std::move(cmd.m_method);
		return *this;
	}

	void setResult(json result)
	{
		m_result = std::move(result);
	}

	void setError(json error)
	{
		m_error = std::move(error);
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

	void checkError()
	{
		if (error().size() != 0) {
			DCORE_THROW(RuntimeError, error().dump());
		}
	}

	enum class TYPE {
		Init,
		Request,
		Result,
		Error
	};

	std::string __toString() const
	{
		return json(*this).dump();
	}

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
	std::string m_jsonRpcVersion = "2.0";
};

// Conversion hooks for Command to json/from json
inline void to_json(json& j, const Command& cmd)
{
	switch (cmd.type()) {
		case Command::TYPE::Request:
			j = json{{"jsonrpc", "2.0"}, {"id", string::toString(cmd.id())}, {"method", cmd.method()}, {"params", cmd.params()}};
			break;
		case Command::TYPE::Result:
			j = json{{"jsonrpc", "2.0"}, {"id", string::toString(cmd.id())}, {"result", cmd.result()}};
			break;
		case Command::TYPE::Error:
			j = json{{"jsonrpc", "2.0"}, {"id", string::toString(cmd.id())}, {"error", cmd.result()}};
			break;
		default:
			DCORE_THROW(RuntimeError, "Cannot convert an un-setup command to json");
	}
}

inline void from_json(const json& j, Command& cmd)
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
