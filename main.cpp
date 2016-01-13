/**
 * Despayre License
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
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

#include <fstream>
#include <string>
#include <boost/locale.hpp>

#include "parser/parser.h"

int main(int argc, char ** argv) try
{
    std::ifstream input{ "./buildfile" };
    std::string buffer_utf8{ std::istreambuf_iterator<char>{ input.rdbuf() }, std::istreambuf_iterator<char>{} };
    std::u32string input_content = boost::locale::conv::utf_to_utf<char32_t>(buffer_utf8);

    auto tokens = reaver::despayre::tokenize(input_content, "buildfile");
    auto parse = reaver::despayre::parse(tokens);
}
catch (reaver::exception & ex)
{
    ex.print(reaver::logger::default_logger());
    return 2;
}
catch (std::exception & ex)
{
    reaver::logger::dlog(reaver::logger::fatal) << ex.what();
    return 1;
}

