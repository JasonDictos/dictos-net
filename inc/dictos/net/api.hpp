#pragma once

namespace dictos::net {

inline auto & GlobalEventMachine()
{
	static EventMachine em;
	return em;
}

inline void dispatch()
{
	GlobalEventMachine().runOne();
}

}
