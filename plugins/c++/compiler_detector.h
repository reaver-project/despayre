/**
 * Despayre License
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation is required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 **/

R"(

#ifndef __cplusplus

despayre: error: not a c++ compiler

#elif __cplusplus > 201402L // above C++14
despayre: c++: 1z
#elif __cplusplus == 201402L // C++14
despayre: c++: 14
#elif __cplusplus == 201103L // C++11
despayre: c++: 11
#elif __cplusplus == 199711L // C++11
despayre: c++: 98
#else // pre-standard
despayre: c++: prehistoric
#endif

#define DOT_CONCAT_IMPL(x, y, z) x ## . ## y ## . ## z
#define DOT_CONCAT(x, y, z) DOT_CONCAT_IMPL(x, y, z)

#if defined(__GNUC__) && !defined(__clang__)

despayre: compiler-id: G++
despayre: compiler-version: DOT_CONCAT(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)

#ifdef __STRICT_ANSI__
#define DESPAYRE_STRICT_COMPILER
#endif

#elif defined(__clang__)

#if defined(__apple_build_version__)
despayre: error: Apple Clang is not supported yet
#else
despayre: compiler-id: Clang++
despayre: compiler-version: DOT_CONCAT(__clang_major__, __clang_minor__, __clang_patchlevel__)
#endif

#ifdef __STRICT_ANSI__
#define DESPAYRE_STRICT_COMPILER
#endif

#else

despayre: compiler-id: unknown

#endif

#ifdef DESPAYRE_STRICT_COMPILER
despayre: compiler-strict: true
#else
despayre: compiler-strict: false
#endif

)"

