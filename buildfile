interesting_target = print(random_variable)
random_variable = "random value"

less_random_variable = "less random value"
not_interesting_target = print(less_random_variable)

// system.language.cxx.version = "c++1z"
// system.language.cxx.flags += "-Wall -Wextra -Wpedantic -Weffc++ -Werror"

// despayre = executable(
//     files("main.cpp"),
//     glob("main/**/*.cpp"),
//     libdespayre
// )

// libdespayre = shared_library(
//     version("1.0.0"),
//     glob("**/*.cpp") - despayre.files - test.files
// )

// test = executable(
//     glob("tests/**/*.cpp"),
//     libdespayre
// )

