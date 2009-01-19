/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: simple_rssreader_app.cpp,v 1.1.1.1 2006/01/16 14:36:45 Hajo Kirchhoff Exp $
 */
#include "stdwx.h"
#include "simple_rssreader_app.h"
#include "rssmainframe.h"
#include "data.h"
#include <litwindow/dataadapter.h>
#include <litwindow/logging.h>
//
#define new DEBUG_NEW   // for debug memory management
using namespace litwindow;
//
extern void InitXmlResource();  // created by wxrc.exe
//
void wx_printer(const TCHAR *l)
{
    wxLogDebug(l);
}
litwindow::static_redirect_tstreambuf wx_redirect(wx_printer);
//
bool SimpleRssReader::OnInit()
{
    wx_redirect.insert(lw_log());
    // add a test channel and a test headline
    Headline firstHeadline;
	firstHeadline.m_title=wxT("The 1st headline");
	firstHeadline.m_body=wxT("<p>This is some news!</p>");
    Channel testChannel;
	testChannel.m_webAddress=wxT("http://www.litwindow.com/rss.xml");
	testChannel.m_title=wxT("Testtitle");
    testChannel.m_headlines.push_back(firstHeadline);
    g_data.m_channels.push_back(testChannel);

    testChannel.m_title=wxT("Second Title");
    testChannel.m_webAddress=wxT("http://www.wxwidgets.org");
    firstHeadline.m_title=wxT("Another headline");
    firstHeadline.m_body=wxT("<p>This is the news for the <b>second</b> headline.");
    testChannel.m_headlines.clear();
    testChannel.m_headlines.push_back(firstHeadline);
    firstHeadline.m_title=wxT("RapidUI rocks!");
    firstHeadline.m_body=wxT("<h1>RapidUI speeds up your work 10-fold!</h><p>Using RapidUI reduces coding drugery and puts the fun back into programming.</p>")
        wxT("<p>For more info, please go to <a href=\"http://www.litwindow.com\">www.litwindow.com</a></p>");
    testChannel.m_headlines.push_back(firstHeadline);
    g_data.m_channels.push_back(testChannel);

    try {
        wxXmlResource::Get()->InitAllHandlers();
        InitXmlResource();
        RssMainFrame *theFrame=new RssMainFrame(0);
        SetTopWindow(theFrame);
        theFrame->Show();
        return true;
    }
    catch (std::runtime_error &e) {
        wxLogError(wxT("std::runtime_error: %s"), e.what());
    }
    return false;
}
IMPLEMENT_APP(SimpleRssReader)

RssReaderData g_data;

BEGIN_ADAPTER(Headline)
    PROP(m_title)
    PROP(m_body)
    PROP(m_published)
    PROP(m_url)
END_ADAPTER()

BEGIN_ADAPTER(Channel)
    PROP(m_webAddress)
    PROP(m_title)
    PROP(m_cacheExpires)
    PROP(m_headlines)
    PROP(m_lastRead)
END_ADAPTER()

BEGIN_ADAPTER(RssReaderData)
    PROP(m_channels)
    PROP(m_refreshAfter)
    PROP(m_nextRefresh)
END_ADAPTER()

IMPLEMENT_ADAPTER_CONTAINER(vector<Headline>)
IMPLEMENT_ADAPTER_CONTAINER(vector<Channel>)
