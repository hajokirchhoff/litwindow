/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: lwbase.cpp,v 1.2 2006/04/04 09:21:33 Hajo Kirchhoff Exp $
*/
#include "stdafx.h"
#include "litwindow/lwbase.hpp"
#include "litwindow/logging.h"
#include <ostream>

using namespace std;
using std::endl;

namespace litwindow {

lwbase_error::~lwbase_error()
{
    lw_err() << "lwbase_error exception " << what() << endl;
}

};
