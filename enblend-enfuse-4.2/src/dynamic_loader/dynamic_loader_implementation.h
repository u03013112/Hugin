/*
 * Copyright (C) 2013-2016 Dr. Christoph L. Spiel
 *
 * This file is part of Enblend.
 *
 * Enblend is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Enblend is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Enblend; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef DYNAMIC_LOADER_IMPLEMENTATION_H_INCLUDED
#define DYNAMIC_LOADER_IMPLEMENTATION_H_INCLUDED


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdexcept>
#include <string>


class DynamicLoaderImplementation
{
public:
    DynamicLoaderImplementation() = delete;

    explicit DynamicLoaderImplementation(const std::string& a_library_name);

    virtual void open() = 0;
    virtual void close() = 0;
    virtual void* resolve(const std::string& a_symbol_name) const = 0;

    const std::string& library_name() const;

    virtual ~DynamicLoaderImplementation() {}

    struct error : public std::runtime_error
    {
        error(const std::string& a_message) : std::runtime_error(a_message) {}
    };

private:
    const std::string name_;
};


#endif // DYNAMIC_LOADER_IMPLEMENTATION_H_INCLUDED


// Local Variables:
// mode: c++
// End:
