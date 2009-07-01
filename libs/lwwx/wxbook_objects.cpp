/* 
* Copyright 2009, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
*/
#include "stdwx.h"
#include <wx/bookctrl.h>
#include <wx/choicebk.h>
#include "wxbook_objects.h"
#include "litwindow/dataadapter.h"
#include "litwindow/wx/rapidui.h"
//#include <sstream>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace litwindow {

    void wxBookCtrlBaseAdapter::OnPageChanged( wxBookCtrlEvent &evt )
    {
        notify_changed();
    }

    void wxBookCtrlBaseAdapter::OnPageChanged( wxChoicebookEvent &evt )
    {
        notify_changed();
    }
    void wxBookCtrlBaseAdapter::notify_changed()
    {
        RapidUI::NotifyChanged(make_aggregate(*m_wnd)["Value"], false);
    }

    void wxBookCtrlBaseAdapter::SetValue( int newValue )
    {
        GetWnd()->SetSelection(newValue);
        notify_changed();
    }

    wxBookCtrlBaseAdapter::wxBookCtrlBaseAdapter( wxBookCtrlBase *w/*=0*/ ) :m_wnd(w)
    {
        //w->Connect(wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED, &wxBookCtrlBaseAdapter::OnPageChanged);
    }
}

using namespace litwindow;

IMPLEMENT_DYNAMIC_CLASS(wxBookCtrlBaseAdapter, lwAdapterBase)
BEGIN_EVENT_TABLE(wxBookCtrlBaseAdapter, lwAdapterBase)
EVT_BOOKCTRL_PAGE_CHANGED(wxID_ANY, OnPageChanged)
EVT_CHOICEBOOK_PAGE_CHANGED(wxID_ANY, OnPageChanged)
END_EVENT_TABLE()

BEGIN_ADAPTER_NO_COPY(wxBookCtrlBaseAdapter)
PROP_I(lwAdapterBase)
PROP_GetSet(int, Value)
END_ADAPTER()

LWL_BEGIN_AGGREGATE_ABSTRACT(wxBookCtrlBase)
PROP_CO(wxBookCtrlBaseAdapter, GetBookCtrlBaseAdapter)
PROP_I(wxControl)
LWL_END_AGGREGATE()
