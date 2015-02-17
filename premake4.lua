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

if not _OPTIONS["install-prefix"] then
	_OPTIONS["install-prefix"] = os.getenv("HOME") .. "/.local/"
end

solution("Hacking")
	configurations({"debug", "release"})
		buildoptions(
			{
				"-std=c99",
				"-I include/",
				"-D_GNU_SOURCE"
			}
		)

		flags(
			{
				"ExtraWarnings"
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

	includedirs(
		{
			"include/"
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

	project("scanner")
		language("C")
		kind("ConsoleApp")

		location("build/bin")
		targetdir("build/bin")

		files(
			{
				"src/hack/hack.c",
				"src/hack/main.c",
				"src/hack/maps.c"
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
				"logger"
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

