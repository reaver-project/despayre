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

#include "despayre/semantics/variable.h"
#include "despayre/semantics/target.h"

std::shared_ptr<reaver::despayre::_v1::target> reaver::despayre::_v1::variable::as_target()
{
    if (type() && type()->is_target_type)
    {
        return std::dynamic_pointer_cast<target>(_shared_this());
    }

    throw std::bad_cast{};
}

std::shared_ptr<reaver::despayre::_v1::variable> reaver::despayre::_v1::variable::operator+(std::shared_ptr<reaver::despayre::_v1::variable> other)
{
    auto it = _operator_plus_call_table.find(other->type());
    if (it != _operator_plus_call_table.end())
    {
        return it->second(_shared_this(), other);
    }

    it = other->_operator_plus_call_table.find(type());
    if (it != other->_operator_plus_call_table.end())
    {
        return it->second(other, _shared_this());
    }

    // delayed_variable solution; it does currently look like a hack, doesn't it?
    it = _shared_this()->_operator_plus_call_table.find(other->type());
    if (it != _shared_this()->_operator_plus_call_table.end())
    {
        return it->second(_shared_this(), other);
    }

    it = other->_shared_this()->_operator_plus_call_table.find(type());
    if (it != other->_shared_this()->_operator_plus_call_table.end())
    {
        return it->second(other, _shared_this());
    }

    // TODO: throw an exception
    assert(0);
}

std::shared_ptr<reaver::despayre::_v1::variable> reaver::despayre::_v1::variable::operator-(std::shared_ptr<reaver::despayre::_v1::variable> other)
{
    auto it = _operator_minus_call_table.find(other->type());
    if (it != _operator_minus_call_table.end())
    {
        return it->second(_shared_this(), other);
    }

    // delayed_variable solution; it does currently look like a hack, doesn't it?
    it = _shared_this()->_operator_minus_call_table.find(other->type());
    if (it != _shared_this()->_operator_minus_call_table.end())
    {
        return it->second(_shared_this(), other);
    }

    // TODO: throw an exception
    assert(0);
}

