#pragma once

namespace dictos::net {

inline auto & GlobalContext()
{
	static Context context;
	return context;
}

inline void dispatch()
{
	GlobalContext().runOne();
}

}
