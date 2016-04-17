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

#include <reaver/mayfly.h>

#include "despayre/parser/parser.h"

namespace
{
    auto parse(const std::u32string & string)
    {
        return reaver::despayre::parse(reaver::despayre::tokenize(string, ""));
    }

    using parse_type = decltype(parse(U""));
}

MAYFLY_BEGIN_SUITE("parser");

MAYFLY_ADD_TESTCASE("empty input", []()
{
    MAYFLY_CHECK(parse(U"") == parse_type{});
});

MAYFLY_ADD_TESTCASE("assignments", []()
{
    using namespace reaver::despayre;

    MAYFLY_CHECK(parse(UR"(a = b)") == parse_type{
        assignment{ {},
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"a", {} } } } },
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"b", {} } } } }
        }
    });

    MAYFLY_CHECK(parse(UR"(a = "b")") == parse_type{
        assignment{ {},
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"a", {} } } } },
            string_node{ {}, { token_type::string, U"b", {} } }
        }
    });

    MAYFLY_CHECK(parse(UR"(a.b.c = "b")") == parse_type{
        assignment{ {},
            id_expression{ {}, {
                identifier{ {}, { token_type::identifier, U"a", {} } },
                identifier{ {}, { token_type::identifier, U"b", {} } },
                identifier{ {}, { token_type::identifier, U"c", {} } },
            }},
            string_node{ {}, { token_type::string, U"b", {} } }
        }
    });

    MAYFLY_CHECK(parse(UR"(a.b.c = d.e)") == parse_type{
        assignment{ {},
            id_expression{ {}, {
                identifier{ {}, { token_type::identifier, U"a", {} } },
                identifier{ {}, { token_type::identifier, U"b", {} } },
                identifier{ {}, { token_type::identifier, U"c", {} } },
            }},
            id_expression{ {}, {
                identifier{ {}, { token_type::identifier, U"d", {} } },
                identifier{ {}, { token_type::identifier, U"e", {} } },
            }}
        }
    });
});

MAYFLY_ADD_TESTCASE("instantiations", []()
{
    using namespace reaver::despayre;

    MAYFLY_CHECK(parse(UR"(a = b())") == parse_type{
        assignment{ {},
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"a", {} } } } },
            instantiation{ {},
                id_expression{ {}, { identifier{ {}, { token_type::identifier, U"b", {} } } } },
                {}
            }
        }
    });

    MAYFLY_CHECK(parse(UR"(a = a.b())") == parse_type{
        assignment{ {},
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"a", {} } } } },
            instantiation{ {},
                id_expression{ {}, {
                    identifier{ {}, { token_type::identifier, U"a", {} } },
                    identifier{ {}, { token_type::identifier, U"b", {} } }
                }},
                {}
            }
        }
    });

    MAYFLY_CHECK(parse(UR"(a = a("abc"))") == parse_type{
        assignment{ {},
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"a", {} } } } },
            instantiation{ {},
                id_expression{ {}, {
                    identifier{ {}, { token_type::identifier, U"a", {} } }
                }},
                {
                    string_node{ {}, { token_type::string, UR"(abc)", {} } }
                }
            }
        }
    });

    MAYFLY_CHECK(parse(UR"(a = a("abc", "def"))") == parse_type{
        assignment{ {},
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"a", {} } } } },
            instantiation{ {},
                id_expression{ {}, {
                    identifier{ {}, { token_type::identifier, U"a", {} } }
                }},
                {
                    string_node{ {}, { token_type::string, UR"(abc)", {} } },
                    string_node{ {}, { token_type::string, UR"(def)", {} } }
                }
            }
        }
    });

    MAYFLY_CHECK(parse(UR"(a = a(abc))") == parse_type{
        assignment{ {},
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"a", {} } } } },
            instantiation{ {},
                id_expression{ {}, {
                    identifier{ {}, { token_type::identifier, U"a", {} } }
                }},
                {
                    id_expression{ {}, { identifier{ {}, { token_type::identifier, U"abc", {} } } } }
                }
            }
        }
    });

    MAYFLY_CHECK(parse(UR"(a = a(abc, x(yz, "uv")))") == parse_type{
        assignment{ {},
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"a", {} } } } },
            instantiation{ {},
                id_expression{ {}, {
                    identifier{ {}, { token_type::identifier, U"a", {} } }
                }},
                {
                    id_expression{ {}, { identifier{ {}, { token_type::identifier, U"abc", {} } } } },
                    instantiation{ {},
                        id_expression{ {}, {
                            identifier{ {}, { token_type::identifier, U"x", {} } }
                        }},
                        {
                            id_expression{ {}, {
                                identifier{ {}, { token_type::identifier, U"yz", {} } }
                            }},
                            string_node{ {}, { token_type::string, UR"(uv)", {} } }
                        }
                    }
                }
            }
        }
    });
});

MAYFLY_ADD_TESTCASE("multiple assignments", []()
{
    using namespace reaver::despayre;

    MAYFLY_CHECK(parse(UR"(a = b c = d)") == parse_type{
        assignment{ {},
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"a", {} } } } },
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"b", {} } } } }
        },
        assignment{ {},
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"c", {} } } } },
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"d", {} } } } }
        }
    });
});

MAYFLY_ADD_TESTCASE("complex expressions", []()
{
    using namespace reaver::despayre;

    MAYFLY_CHECK(parse(UR"(a = b + c)") == parse_type{
        assignment{ {},
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"a", {} } } } },
            complex_expression{ {},
                id_expression{ {}, { identifier{ {}, { token_type::identifier, U"b", {} } } } },
                {
                    operation{ {}, operation_type::addition, { id_expression{ {}, { identifier{ {}, { token_type::identifier, U"c", {} } } } } } }
                }
            }
        }
    });

    MAYFLY_CHECK(parse(UR"(a = b - c)") == parse_type{
        assignment{ {},
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"a", {} } } } },
            complex_expression{ {},
                id_expression{ {}, { identifier{ {}, { token_type::identifier, U"b", {} } } } },
                {
                    operation{ {}, operation_type::removal, { id_expression{ {}, { identifier{ {}, { token_type::identifier, U"c", {} } } } } } }
                }
            }
        }
    });

    MAYFLY_CHECK(parse(UR"(a = b + c + d)") == parse_type{
        assignment{ {},
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"a", {} } } } },
            complex_expression{ {},
                id_expression{ {}, { identifier{ {}, { token_type::identifier, U"b", {} } } } },
                {
                    operation{ {}, operation_type::addition, { id_expression{ {}, { identifier{ {}, { token_type::identifier, U"c", {} } } } } } },
                    operation{ {}, operation_type::addition, { id_expression{ {}, { identifier{ {}, { token_type::identifier, U"d", {} } } } } } }
                }
            }
        }
    });

    MAYFLY_CHECK(parse(UR"(a = b + c - d)") == parse_type{
        assignment{ {},
            id_expression{ {}, { identifier{ {}, { token_type::identifier, U"a", {} } } } },
            complex_expression{ {},
                id_expression{ {}, { identifier{ {}, { token_type::identifier, U"b", {} } } } },
                {
                    operation{ {}, operation_type::addition, { id_expression{ {}, { identifier{ {}, { token_type::identifier, U"c", {} } } } } } },
                    operation{ {}, operation_type::removal, { id_expression{ {}, { identifier{ {}, { token_type::identifier, U"d", {} } } } } } }
                }
            }
        }
    });
});

MAYFLY_END_SUITE;

