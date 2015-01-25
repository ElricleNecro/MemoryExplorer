newoption(
	{
		trigger="install-prefix",
		value="DIR",
		description="Directory used to install bin, lib and share directory.",
	}
)

if not _OPTIONS["install-prefix"] then
	_OPTIONS["install-prefix"] = os.getenv("HOME") .. "/.local/"
end

print(_OPTIONS)

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
				"-g"
			}
		)

	includedirs(
		{
			"include/"
		}
	)

	project("scanner")
		language("C")
		kind("ConsoleApp")

		location("build/bin")
		targetdir("build/bin")

		files(
			{
				"src/hack/*.c"
			}
		)

		links(
			{
				"readline"
			}
		)
		-- libdirs(
			-- {
				-- "build/lib"
			-- }
		-- )

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

