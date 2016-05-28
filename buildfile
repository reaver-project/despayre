cxx.version = "c++1z"
cxx.flags = "-Wall"
cxx.gcc.flags = ""
cxx.clang.flags = "-Wextra -Wpedantic -Weffc++ -Werror"

modules.cxx = import("c++", cxx)

main_sources = files("main.cpp") + glob("main/**/*.cpp")
lib_sources = glob("**/*.cpp") - main_sources - test_sources //- plugins.sources
test_sources = glob("tests/**/*.cpp")

// plugins = include("plugins")

despayre = executable(
    "despayre",
    main_sources
//     libdespayre
)

libdespayre = shared_library(
    "despayre",
//     version("1.0.0"),
    lib_sources
)

test = executable(
    "despayre-test",
    test_sources
//     libdespayre
)

all = aggregate(
    despayre,
    libdespayre,
    test
)

