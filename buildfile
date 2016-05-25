interesting_target = debug_print(namespace.random_variable)
namespace.random_variable = "random value"

namespace.less_random_variable = "less random value"
not_interesting_target = debug_print(namespace.less_random_variable)

// type constructor aliases also work before their definition...
random = namespace.aggregate(interesting_target)

// type constructors are first-class variables
namespace.aggregate = aggregate

namespace.aggregate1 = aggregate(interesting_target, not_interesting_target)
aggregate2 = namespace.aggregate(interesting_target, not_interesting_target)

top_level = aggregate(namespace.aggregate1, aggregate2, combined.aggregate, another_print)

combined.aggregate = aggregate(combined.print1, combined.print2)
combined.print1 = debug_print("abc" + "def" + "ghi")
combined.to_be_printed2 = "fde" + "cba"
combined.print2 = debug_print(combined.to_be_printed2)

another_print = debug_print(string1 + string2 + string3)
string1 = "foo"
string2 = "bar"
string3 = "baz"

// the actual buildfile for despayre itself
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

// libdespayre = shared_library(
//     "despayre",
//     version("1.0.0"),
//     lib_sources,
// )

test = executable(
    "despayre-test",
    test_sources
//     libdespayre
)

all = aggregate(
    despayre,
    test
)

