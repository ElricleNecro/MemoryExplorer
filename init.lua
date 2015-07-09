Event = open_event()

function test_modif(a_addr, stop_addr)
	a_value = ev:scan(a_addr, 4, "int")
	stop_value = ev:scan(stop_addr, 4, "int")

	print("a = ", a_value)
	print("stop = ", stop_value)

	print("setting a to ", stop_value - 2)

	ev:write(a_addr, 4, "int", stop_value - 2)

	return ev:scan(a_addr, 4, "int")
end

