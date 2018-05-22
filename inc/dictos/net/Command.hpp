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

		if (_param != j.end()) {
			m_params = std::move(*_param);
			m_type = TYPE::Request;
		}
		if (_error != j.end()) {
			m_error = std::move(*_error);
			m_type = TYPE::Error;
		}
		if (_method != j.end()) {
			m_method = std::move(_method->get<std::string>());
			m_type = TYPE::Request;
		}
		if (_id != j.end())
			m_id = _id->get<Uuid>();	 
		if (_result != j.end()) {
			m_result = std::move(*_result);
			m_type = TYPE::Result;
		}

		// If we've constructed as a request with no id, generate one here implicitly
		if (!m_id && type() == TYPE::Request) {
			m_id = Uuid::create();
		}
	}

	Command(std::string_view method, json params) :
		m_method(method.begin(), method.end()),
		m_params(std::move(params)),
		m_type(TYPE::Request)
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
		m_method(std::move(cmd.m_method)),
		m_type(cmd.m_type)
	{
		cmd.m_type = TYPE::Init;
	}

	Command & operator = (Command &&cmd)
	{
		m_id = std::move(cmd.m_id);
		m_params = std::move(cmd.m_params);
		m_result = std::move(cmd.m_result);
		m_error = std::move(cmd.m_error);
		m_jsonRpcVersion = std::move(cmd.m_jsonRpcVersion);
		m_method = std::move(cmd.m_method);
		m_type = cmd.m_type;
		cmd.m_type = TYPE::Init;
		return *this;
	}

	void setResult(json result)
	{
		m_result = std::move(result);
		m_type = TYPE::Result;
	}

	void setError(json error)
	{
		m_error = std::move(error);
		m_type = TYPE::Error;
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

	TYPE type() const { return m_type; }

	bool operator < (const Command &command) const
	{
		return m_id < command.m_id;
	}

protected:
	std::string m_method;
	json m_params, m_result, m_error;
	Uuid m_id;
	std::string m_jsonRpcVersion = "2.0";
	TYPE m_type = TYPE::Init;
};

// Conversion hooks for Command to json/from json
inline void to_json(json& j, const Command& cmd)
{
	switch (cmd.type()) {
		case Command::TYPE::Request:
			j = json{{"jsonrpc", "2.0"}, {"id", cmd.id()}, {"method", cmd.method()}, {"params", cmd.params()}};
			break;
		case Command::TYPE::Result:
			j = json{{"jsonrpc", "2.0"}, {"id", cmd.id()}, {"result", cmd.result()}};
			break;
		case Command::TYPE::Error:
			j = json{{"jsonrpc", "2.0"}, {"id", cmd.id()}, {"error", cmd.result()}};
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
		cmd.id() = j.at("id").get<Uuid>();
	} else if (j.find("result") != j.end()) {
		cmd.result() = j.at("params").get<json>();
		cmd.id() = j.at("id").get<Uuid>();
	} else if (j.find("error") != j.end()) {
		cmd.error() = j.at("error").get<json>();
		cmd.id() = j.at("id").get<Uuid>();
	} else {
		DCORE_THROW(RuntimeError, "Invalid json payload");
	}
}

}
