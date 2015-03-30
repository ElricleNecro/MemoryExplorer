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

