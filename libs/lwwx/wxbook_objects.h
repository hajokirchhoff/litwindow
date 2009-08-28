/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: wxmisc_objects.h,v 1.3 2007/04/12 08:36:52 Hajo Kirchhoff Exp $
*/
#ifndef _LW_WXBOOK_OBJECTS_
#define _LW_WXBOOK_OBJECTS_

#pragma once

#include <litwindow/lwbase.hpp>
#include <litwindow/dataadapter.h>
#include <wx/bookctrl.h>
#include <wx/choicebk.h>
#include "./base_objects.h"

#ifndef x_DOC_DEVELOPER

class wxBookCtrlBase;
class wxBookCtrlEvent;

namespace litwindow {

    class wxBookCtrlBaseAdapter:public lwAdapterBase
    {
        wxBookCtrlBase *m_wnd;
        void OnPageChanged(wxBookCtrlEvent &evt);
        void OnChoicePageChanged(wxChoicebookEvent &evt);
        void notify_changed();
    public:
        wxBookCtrlBaseAdapter(wxBookCtrlBase *w=0);
        wxBookCtrlBase *GetWnd() const { return m_wnd; }
        int GetValue() const { return m_wnd->GetSelection(); }
        void SetValue(int newValue);
        DECLARE_DYNAMIC_CLASS_NO_COPY(wxBookCtrlBaseAdapter)
        DECLARE_EVENT_TABLE()
    };

    inline wxBookCtrlBaseAdapter *GetBookCtrlBaseAdapter(wxBookCtrlBase *w) { return GetAdapter<wxBookCtrlBaseAdapter>(w, CLASSINFO(wxBookCtrlBaseAdapter)); }

};

#endif

#endif

