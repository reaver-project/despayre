# Despayre, the build system you always wanted

Despayre is the build system of the Reaver Project. Unlike other build systems,
it doesn't devise its own domain specific language to write "makefiles" in, and
just provides a set of C++ libraries that let you write your build system in it,
complete with all possible language features you may want.

The choice of C++ over scripting languages like Python or Ruby was conscious. This
way, with the specific instance of the build system being "cached" in a shared
library, you can write various tools that directly call Despayre's API to build
the project, including editors integrated with the build system.

The first time you run Despayre on a directory that has proper files in its
directory structure, it will build the specific library responsible for building
the project itself. Following times it will just load the library and build your
project. Dependencies for the "makefiles" are tracked automatically, so they will
always be rebuilt automatically.

The name of Despayre is derived from the name of a penial world in the Star Wars
universe, a place where the original Death Star was constructed.

