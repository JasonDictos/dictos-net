#pragma once

namespace dictos::net {

/**
 * A partway basic json rpc clone representing a json method with params. Although
 * it uses json as its core translation type, other serialization targets may be
 * possible, hence the name of this class is kept generic, however json provides
 * basic types most easily represented across every major language out there so
 * its a good type system for any rpc serialization type.
 */
class Command
{
public:
	Command() = default;

	Command(std::string_view method) :
		m_method(method.begin(), method.end())
	{
	}

	Command(std::string_view method, json params) :
		m_method(method.begin(), method.end()), m_params(std::move(params))
	{
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
		return *this;
	}

	Command & operator = (Command &&cmd)
	{
		m_method = std::move(cmd.m_method);
		m_params = std::move(cmd.m_params);
		return *this;
	}

	std::string getMethod() const { return m_method; }

	const json &getParams() const { return m_params; }
	json &getParams() { return m_params; }

	auto & operator[] (const std::string &key) { return m_params[key]; }

protected:
	std::string m_method;
	json m_params;
};

inline void to_json(json& j, const Command& p)
{
	j = json{{"method", p.getMethod()}, {"params", p.getParams()}};
}

inline void from_json(const json& j, Command& cmd)
{
	cmd = Command(
		j.at("method").get<std::string>(),
		j.at("params").get<json>()
	);
}

}
