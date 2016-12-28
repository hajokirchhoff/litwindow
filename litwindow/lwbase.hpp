/*
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library
 * distribution, file LICENCE.TXT
 * $Id: lwbase.hpp,v 1.1 2006/04/04 09:22:11 Hajo Kirchhoff Exp $
 */
#ifndef _LITWINDOW_LWBASE_HPP
#define _LITWINDOW_LWBASE_HPP

#include "./config.hpp"
#include "./tchar.h"
#include <stdexcept>
#include <string>

namespace litwindow {

      /** Basic exception class for litwindow exceptions.
            All exceptions thrown by the litwindow library are lwbase_errors unless otherwise noted.
            */

class lwbase_error:public std::runtime_error
{
public:
    LWBASE_API lwbase_error(const std::string &message)
        :std::runtime_error(message)
    {
    }
    LWBASE_API ~lwbase_error() throw();
};

};

#endif

#ifdef _MSC_VER
#pragma once
#endif
