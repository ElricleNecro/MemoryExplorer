Event = {}
setmetatable(Event, Event)
function Event.__call()
	local self = {}
	setmetatable(self, Event)
	return self
end

Event.__index = Event_index
Event.__newindex = Event_newindex

ev = Event()

function Lquit()
	ev.quit = true
end

CBind = {}
setmetatable(CBind, CBind)
function CBind.__call()
	local self = {}
	setmetatable(self, CBind)
	return self
end

CBind.__index = function(table, key)
	return function(...)
		return C_call(key, ...)
	end
end

C = CBind()

function test_modif(a_addr, stop_addr)
	a_value = Scanner(ev, a_addr, 4, "int")
	stop_value = Scanner(ev, stop_addr, 4, "int")

	print("a = ", a_value)
	print("stop = ", stop_value)

	print("setting a to ", stop_value - 2)

	Writer(ev, a_addr, 4, "int", stop_value - 2)

	return Scanner(ev, a_addr, 4, "int")
end

