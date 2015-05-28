newoption(
	{
		trigger="install-prefix",
		value="DIR",
		description="Directory used to install bin, lib and share directory.",
	}
)

newoption(
	{
		trigger="with-readline",
		description="Use of readline for the prompt.",
	}
)

newoption(
	{
		trigger="with-readv",
		description="Use of process_vm_readv instead of ptrace to read memory.",
	}
)

newoption {
	trigger     = "ptrace-use",
	value       = "USE",
	description = "Choose what to use to read memory with ptrace.",
	allowed = {
		{ "pure",   "Pure ptrace call." },
		{ "lseek",  "A combination of lseek and read in the mem file (Linux Only)." },
		{ "pread",  "Use of pread to read the mem file (Linux Only)." }
	}
}

if not _OPTIONS["install-prefix"] then
	_OPTIONS["install-prefix"] = os.getenv("HOME") .. "/.local/"
end

if not _OPTIONS["ptrace-use"] then
	_OPTIONS["ptrace-use"] = "pure"
end

if _OPTIONS["ptrace-use"] == "pure" then
	define_ptrace = "USE_PURE_PTRACE"
elseif _OPTIONS["ptrace-use"] == "lseek" then
	define_ptrace = "USE_lseek_read"
else
	define_ptrace = "USE_pread"
end

solution("Hacking")
	configurations({"debug", "release"})
		buildoptions(
			{
				"-std=c99"
			}
		)

		flags(
			{
				"ExtraWarnings"
			}
		)

		defines(
			{
				"_GNU_SOURCE"
			}
		)

	configuration("release")
		buildoptions(
			{
				"-O3"
			}
		)

	configuration("debug")
		buildoptions(
			{
				"-g3"
			}
		)
		flags(
			{
				"Symbols"
			}
		)

	includedirs(
		{
			"include/",
			".submodule/LuaAutoC/",
			".submodule/ParseArgsC/include/"
		}
	)

	project("dict")
		language("C")
		kind("SharedLib")

		location("build/lib")
		targetdir("build/lib")

		files(
			{
				"src/dict/*.c"
			}
		)

	project("logger")
		language("C")
		kind("SharedLib")

		location("build/lib")
		targetdir("build/lib")

		files(
			{
				"src/logger/*.c"
			}
		)

	project("ParseArgsC")
		language("C")
		kind("SharedLib")

		location("build/lib")
		targetdir("build/lib")

		files(
			{
				".submodule/ParseArgsC/src/Parser.c"
			}
		)

	project("LuaAutoC")
		language("C")
		kind("StaticLib")

		location("build/lib")
		targetdir("build/lib")

		files(
			{
				".submodule/LuaAutoC/lautoc.c"
			}
		)

	project("scanner")
		language("C")
		kind("ConsoleApp")

		location("build/bin")
		targetdir("build/bin")

		files(
			{
				"src/hack/hack.c",
				"src/hack/main.c",
				"src/hack/maps.c",
				"src/hack/lua_decl.c"
			}
		)

		defines(
			{
				define_ptrace,
				"LUA_COMPAT_APIINTCASTS"
			}
		)

		configuration("with-readv")
			defines(
				{
					"USE_vm_readv"
				}
			)

		configuration("with-readline")
			files(
				{
					"src/hack/readline.c"
				}
			)
			defines(
				{
					"USE_readline"
				}
			)
			links(
				{
					"readline"
				}
			)

		libdirs(
			{
				"build/lib"
			}
		)

		links(
			{
				"ParseArgsC",
				"LuaAutoC",
				"logger",
				"dict",
				"lua",
				"m"
			}
		)

	project("tester")
		language("C")
		kind("ConsoleApp")

		location("build/bin")
		targetdir("build/bin")

		files(
			{
				"src/timer/*.c"
			}
		)

