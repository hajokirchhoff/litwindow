/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: lwwx.cpp,v 1.2 2006/04/04 09:21:33 Hajo Kirchhoff Exp $
*/
#include "stdwx.h"

#include "litwindow/wx/lwwx.h"
#include "litwindow/logging.h"
#include <fstream>
#include <wx/datetime.h>
#include <wx/filename.h>

namespace litwindow {

struct get_lwlog_dumper:public wrapping_tstreambuf::dumper_t
{
    tostream &get()
    {
        if (!out.is_open())
            out.open(t2string(m_file_name).c_str(), ios::app);
        return out << endl << wxT("************************ ") << m_title << wxT(" ***************************") << endl << wxDateTime::Now().Format(_T("%c")) << endl;
    }
    tstring m_file_name;
    tstring m_title;
    tofstream out;

    get_lwlog_dumper(const TCHAR *file_name, const TCHAR *title)
        :m_file_name(file_name), m_title(title)
    {
    }
};

struct set_lwlog_dumper
{

    static void to_wxlog(const TCHAR *l)
    {
        try {
            wxLogDebug(l);
        }
        catch (...) {
        }
    }

    set_lwlog_dumper()
    {
        wxString fn;
        {
            wxFileName fName(wxT("lwlog_dump.txt"));
            fName.MakeAbsolute();
            fn=fName.GetFullPath();
        }
        wxLogNull nul;
        size_t backups;
        const size_t no_of_backups=3;
        wxRemoveFile(fn+wxString::Format(wxT(".%d"), no_of_backups));
        for (backups=no_of_backups; backups>0; --backups) {
            wxRenameFile(fn+wxString::Format(wxT(".%d"), backups-1), fn+wxString::Format(wxT(".%d"), backups));
        }
        wxRenameFile(fn, fn+wxT(".0"));
        //lw_log().set_dumper(new get_lwlog_dumper(fn.c_str(), wxT("lw_log")));
        //lw_err().set_dumper(new get_lwlog_dumper(fn.c_str(), wxT("#### lw_err ####")));
    }

    void send_log_to_wxdebug(bool enabled)
    {
        static static_redirect_tstreambuf wxredirect(to_wxlog);
        static int send_log_to_count=0;
        if (enabled) {
            if (send_log_to_count++==0)
                wxredirect.insert(lw_log());
        } else {
            if (--send_log_to_count==0)
                wxredirect.remove();
        }
    }
} _set_lwlog_dumper;

void enable_log_to_wxdebug(bool enabled)
{
    _set_lwlog_dumper.send_log_to_wxdebug(enabled);
}

};
