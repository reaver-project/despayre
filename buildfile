interesting_target = debug.print(namespace.random_variable)
namespace.random_variable = "random value"

namespace.less_random_variable = "less random value"
not_interesting_target = debug.print(namespace.less_random_variable)

// type constructors are first-class variables
namespace.aggregate = aggregate

namespace.aggregate1 = aggregate(interesting_target, not_interesting_target)
aggregate2 = namespace.aggregate(interesting_target, not_interesting_target)

top_level = aggregate(namespace.aggregate1, aggregate2, combined.aggregate)

combined.aggregate = aggregate(combined.print1, combined.print2)
combined.print1 = debug.print("abc" + "def" + "ghi")
combined.to_be_printed2 = "fde" + "cba"
combined.print2 = debug.print(combined.to_be_printed2)

// the actual buildfile for despayre itself
system.language.cxx.version = "c++1z"
system.language.cxx.flags = "-Wall"
system.compiler.gcc.flags = ""
system.compiler.clang.flags = "-Wall -Wextra -Wpedantic -Weffc++ -Werror"

// main_sources = files("main.cpp") + glob("main/**/*.cpp")
// lib_sources = glob("**/*.cpp") - main_sources - test_sources
// test_sources = glob("tests/**/*.cpp")

// despayre = executable(
//     files("main.cpp"),
//     main_sources,
//     libdespayre
// )

// libdespayre = shared_library(
//     version("1.0.0"),
//     lib_sources,
// )

// test = executable(
//     test_sources,
//     libdespayre
// )

