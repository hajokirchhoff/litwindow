/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: data.h,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#pragma once

#include <vector>
#include "wx/datetime.h"
#include "litwindow/tstring.h"

using namespace std;
using litwindow::tstring;

struct Headline
{
    wxString    m_title;        // the headline title
    wxString    m_body;         // the news in html
    wxDateTime  m_published;    // date/time when this headline was published
    tstring     m_url;          // an associated URL
};

struct Channel
{
    tstring             m_webAddress;   // url of the channel
    wxString            m_title;        // the title of this channel
    wxTimeSpan          m_cacheExpires; // timespan between two refreshes
    vector<Headline>    m_headlines;    // the list of headlines
    wxDateTime          m_lastRead;     // date/time when this channel was last read
};

struct RssReaderData
{
    vector<Channel> m_channels;         // store all channels
    wxTimeSpan      m_refreshAfter;     // timespan between two refreshes
    wxDateTime      m_nextRefresh;      // time of next refresh
};

extern RssReaderData g_data;
