function test_modif(a_addr, stop_addr)
	local a_value, stop_value

	a_value = ev:scan(a_addr, 4, "int")
	stop_value = ev:scan(stop_addr, 4, "int")

	print("a = ", a_value)
	print("stop = ", stop_value)

	print("setting a to ", stop_value - 2)

	ev:write(a_addr, 4, "int", stop_value - 2)

	return ev:scan(a_addr, 4, "int")
end

function initialize(a_addr, s_addr, d_addr)
	local stack = {}
	local heap = {}
	local code = {}
	local var = {}

	stack.start, stack.fin = ev:get_map("stack")
	heap.start, heap.fin = ev:get_map("heap")
	code.start, code.fin = ev:get_map("code")

	var.a, var.s, var.d = a_addr, s_addr, d_addr

	return stack, heap, code, var
end
