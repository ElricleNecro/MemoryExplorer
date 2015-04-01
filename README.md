# MemoryExplorer
A little tool to understand how memory and program work under Linux!

At this point, this program is able to read and write another process memory using multiple methods.

## Compilation
==============

```
$ git submodule init
$ git submodule update
```

```
$ premake4 [All options you want] [type of project to build]
```
The available building option are:
 - `--with-readline`: build with readline support (you should...).
 - `--with-readv`: if you prefer to use the `process_vm_*` method to read write memory (Linux >3.2 Only).
 - `--ptrace-use=METHOD`: if you prefer ptrace. The argument can be:
   - `pure`: use only ptrace call (not fully working right now).
   - `lseek`: use a combination of `lseek` and `read`/`write` to read directly the `/proc/$(pid)/mem` file (Linux Only).
   - `pread`: use `pread` and `pwrite` call to read directly the `/proc/$(pid)/mem` file (Linux Only).
 - `--install-prefix`: installation prefix. Not yet used.

Mac and UNIX users can only use the `--ptrace-use=pure` option. I still need to find an equivalent of `/proc/$(pid)/maps` to get the memory map of the process (to be able to know where are the heap, stack & co section).

I personnaly build it, under Linux, with:
```
$ premake4 --with-readline --ptrace-use=pread gmake
```

Then, you just need to do:
```
$ make
```

## Testing
==========

To test the program, got to the project directory, then:
```
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:build/lib
$ build/bin/scanner build/bin/tester
```

You should see some line containing some memory addresses, print by `tester`. Get the address of `a` and `stop` and then:
```
> a_address = (the address you've seen)
> stop_address = (the address you've seen)
> test_modif(a_address, stop_address)
```

You can look at what this function do by looking at the `init.lua` file.

## To come
==========

- Enhanced LUA interpreter (this one is very basic).
- make the pure ptrace version working.
- make a cleaner interface to the C part (for example, there is no way to access to the memory map I've cronstruct).
- prepare the code injection part of the program (playing with assembler and disassembler code)!

