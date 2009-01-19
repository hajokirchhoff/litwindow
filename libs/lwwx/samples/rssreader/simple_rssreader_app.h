/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: simple_rssreader_app.h,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#pragma once
class SimpleRssReader:public wxApp
{
public:
    bool OnInit();
};
DECLARE_APP(SimpleRssReader)
