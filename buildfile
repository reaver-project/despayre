interesting_target = print(random_variable)
random_variable = "random value"

less_random_variable = "less random value"
not_interesting_target = print(less_random_variable)

aggregate1 = aggregate(interesting_target, not_interesting_target)
aggregate2 = aggregate(interesting_target, not_interesting_target)

top_level = aggregate(aggregate1, aggregate2)

// system.language.cxx.version = "c++1z"
// system.language.cxx.flags += "-Wall -Wextra -Wpedantic -Weffc++ -Werror"

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

