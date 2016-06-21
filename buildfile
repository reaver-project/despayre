cxx.version = "c++1z"

cxx.flags = "-Wall"
cxx.gcc.flags = ""
cxx.clang.flags = "-Wextra -Wpedantic -Weffc++ -Werror"

cxx.ldflags = "-pthreads"
cxx.gcc.ldflags = ""
cxx.clang.ldflags = ""

modules.cxx = import("c++", cxx)

main_sources = files("main.cpp") + glob("main/**/*.cpp")
lib_sources = glob("**/*.cpp") - main_sources - test_sources - plugins.sources
test_sources = glob("tests/**/*.cpp")

// plugins = include("plugins")
plugins.cxx_files = glob("plugins/c++/**/*.cpp")
plugins.cxx = shared_library(
    "despayre.c++",
    plugins.cxx_files
)

plugins.sources = plugins.cxx_files
plugins.all = aggregate(
    plugins.cxx
)

despayre = executable(
    "despayre",
    main_sources,
    libdespayre
    //library("boost_filesystem"),
    //library("boost_system"),
    //library("dl")
)

libdespayre = shared_library(
    "despayre",
//     version("1.0.0"),
    lib_sources
)

test = executable(
    "despayre-test",
    test_sources,
    libdespayre
    //library("boost_filesystem"),
    //library("boost_system"),
    //library("dl"),
    //library("boost_program_options"),
    //library("boost_iostream")
)

all = aggregate(
    despayre,
    plugins.all
)

// vim: set filetype=c:

