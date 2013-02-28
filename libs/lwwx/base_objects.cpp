/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: base_objects.cpp,v 1.7 2007/10/26 11:53:01 Hajo Kirchhoff Exp $
 */
#include "stdwx.h"
using namespace std;

#include <wx/gdicmn.h>
#include <wx/datetime.h>
#include <wx/html/htmlwin.h>
#include <wx/spinctrl.h>
#include <wx/datectrl.h>
#include <wx/calctrl.h>
#include "litwindow/dataadapter.h"
#include "litwindow/wx/rapidUI.h"
#include "litwindow/logging.h"
#include "base_objects.h"
#include "wxlist_objects.h"
#include "wxtext_objects.h"
#include "wxbutton_objects.h"
#include "wxmisc_objects.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef DOXYGEN_INVOKED

using namespace litwindow;

template <>
tstring litwindow::converter<wxString>::to_string(const wxString &s)
{
	MessageBox(
		NULL,
		(LPCWSTR)"lwwx library", (LPCWSTR)"base_objects.cpp@tstring litwindow::converter<wxString>::to_string(const wxString &s): (!)maybe porting critical", 
		MB_OK | MB_ICONINFORMATION);

   //return tstring(s.c_str());
	return s.wc_str();
}

template <>
tstring litwindow::converter<wxCStrData>::to_string(const wxCStrData &s)
{
	MessageBox(
		NULL,
		(LPCWSTR)"lwwx library", (LPCWSTR)"base_objects.cpp@tstring litwindow::converter<wxCStrData>::to_string(const wxCStrData &s): (!)maybe porting critical", 
		MB_OK | MB_ICONINFORMATION);

	return s.AsWChar();
}

template <>
size_t litwindow::converter<wxString>::from_string(const tstring &newValue, wxString &v)
{
    v=newValue.c_str();
    return v.size();
}

IMPLEMENT_ADAPTER_TYPE(wxString)

template <>
tstring litwindow::converter<wxSize>::to_string(const wxSize &v)
{
    tostringstream s;
    s << wxT("(") << v.GetWidth() << wxT(", ") << v.GetHeight() << wxT(")");
    return s.str();
}

template <>
tstring litwindow::converter<wxDateTime>::to_string(const wxDateTime &d)
{
	MessageBox(
		NULL,
		(LPCWSTR)"lwwx library", (LPCWSTR)"base_objects.cpp@tstring litwindow::converter<wxDateTime>::to_string(const wxDateTime &d): (!)maybe porting critical", 
		MB_OK | MB_ICONINFORMATION);

    return d.IsValid() ? d.Format(wxT("%Y-%m-%d %H:%M:%S")).t_str() : _T("invalid_date");
}

template <>
size_t litwindow::converter<wxDateTime>::from_string(const tstring &newValue, wxDateTime &v)
{
	if (newValue==wxT("invalid_date"))
		v=wxDateTime();
	else {
		static const TCHAR *fmts[]={
			wxT("%Y-%m-%d %H:%M:%S"),
			wxT("%H:%M")
		};
		const wxDateTime currentTime(v);
		size_t i;
		for (i=0; i<WXSIZEOF(fmts) && v.ParseFormat(newValue.c_str(), fmts[i], currentTime)==NULL; ++i)
			;
		if (i==WXSIZEOF(fmts))
			throw lwbase_error("invalid wxDateTime format");
	}
	return sizeof(v);
}

IMPLEMENT_ADAPTER_TYPE(wxDateTime)

template <>
tstring litwindow::converter<wxTimeSpan>::to_string(const wxTimeSpan &s)
{
	MessageBox(
		NULL,
		(LPCWSTR)"lwwx library", (LPCWSTR)"base_objects.cpp@tstring litwindow::converter<wxTimeSpan>::to_string(const wxTimeSpan &s): (!)maybe porting critical", 
		MB_OK | MB_ICONINFORMATION);

    wxLongLong v=s.GetValue();
    wxString temp;
    temp.Format(wxT("%08x%08x"), v.GetHi(), v.GetLo());
    return temp.t_str();
}

template <>
size_t litwindow::converter<wxTimeSpan>::from_string(const tstring &newValue, wxTimeSpan &v)
{
    long hi, lo;
    if (_stscanf(newValue.substr(0, 8).c_str(), wxT("%08x"), &hi)!=1 || 
	    _stscanf(newValue.substr(8, 8).c_str(), wxT("%08x"), &lo)!=1) {
	    wxDateTime t;
	    try {
		    make_accessor(t).from_string(newValue);
	    }
	    catch (...) {
		    throw lwbase_error("invalid wxTimeSpan format");
	    }
    }
    wxLongLong longValue(hi, lo);
    v=longValue;
    return sizeof (v);
}

IMPLEMENT_ADAPTER_TYPE(wxTimeSpan)

    //-----------------------------------------------------------------------------------------------------------//
    //-----------------------------------------------------------------------------------------------------------//
    //NOTE: If you add new data adapters or new properties to existing adapters, don't forget to update the
    // defaultPropertyEventTypes table below as well.
BEGIN_ADAPTER(wxSize)
PROP_GetSet(int, Height)
PROP_GetSet(int, Width)
END_ADAPTER()

BEGIN_ADAPTER(wxPoint)
    PROP(x)
    PROP(y)
END_ADAPTER()

LWL_BEGIN_AGGREGATE(wxRect)
PROP_GetSet(int, X)
PROP_GetSet(int, Y)
PROP_GetSet(int, Height)
PROP_GetSet(int, Width)
LWL_END_AGGREGATE()

BEGIN_ADAPTER_NO_COPY(wxWindow)
    PROP_RW_WITH_SIGNATURE(wxSize, wxSize (wxWindow::*)() const, wxWindow::GetSize, void (wxWindow::*)(const wxSize&), wxWindow::SetSize, "Size")
    PROP_RW_FUNC(bool, IsEnabled, Enable, "Enabled")
    PROP_RW_FUNC(bool, IsShown, Show, "Shown")
END_ADAPTER()

BEGIN_ADAPTER_NO_COPY(wxControl)
    PROP_I(wxWindow)
    PROP_GetSet(wxString, Label)
    PROP_RW_FUNC(wxString, GetLabel, SetLabel, "Value")
END_ADAPTER()

BEGIN_ADAPTER_NO_COPY(wxRadioBox)
    PROP_CO(wxRadioBoxAdapter, GetRadioBoxAdapter)
    PROP_I(wxControl)
END_ADAPTER()

BEGIN_ADAPTER_NO_COPY(wxCheckBox)
    PROP_CO(wxCheckBoxAdapter, GetCheckBoxAdapter)
    PROP_I(wxControl)
END_ADAPTER()

BEGIN_ADAPTER_NO_COPY(wxButton)
    PROP_CO(wxButtonAdapter, GetButtonAdapter)
    PROP_I(wxControl)
END_ADAPTER()

BEGIN_ADAPTER_NO_COPY(wxRadioButton)
    PROP_CO(wxRadioButtonAdapter, GetRadioButtonAdapter)
    PROP_I(wxControl)
END_ADAPTER()

BEGIN_ADAPTER_NO_COPY(wxTextCtrl)
    PROP_CO(wxTextCtrlAdapter, GetTextCtrlAdapter)
    PROP_I(wxControl)
    //PROP_RW_FUNC(wxString, GetValue, SetValue, "Value")
END_ADAPTER()

LWL_BEGIN_AGGREGATE_NO_COPY(wxStaticText)
	PROP_CO(wxStaticTextAdapter, GetStaticTextAdapter)
	PROP_I(wxControl)
LWL_END_AGGREGATE()

BEGIN_ADAPTER_NO_COPY(wxListBox)
    PROP_CO(wxListBoxAdapter, GetListBoxAdapter)
    PROP_I(wxControl)
END_ADAPTER()

BEGIN_ADAPTER_NO_COPY(wxComboBox)
    PROP_CO(wxComboBoxAdapter, GetComboBoxAdapter)
    PROP_I(wxControl)
    //PROP_GetSet(wxString, Value)
END_ADAPTER()

BEGIN_ADAPTER_NO_COPY(wxChoice)
    PROP_CO(wxChoiceAdapter, GetChoiceAdapter)
    PROP_I(wxControl)
END_ADAPTER()

BEGIN_ADAPTER_NO_COPY(wxSpinCtrl)
    PROP_CO(wxSpinCtrlAdapter, GetSpinCtrlAdapter)
    PROP_I(wxControl)
END_ADAPTER()

BEGIN_ADAPTER_NO_COPY(wxFrame)
    PROP_I(wxWindow)
    PROP_GetSet(wxString, Title)
END_ADAPTER()

BEGIN_ADAPTER_NO_COPY(wxDatePickerCtrl)
	PROP_CO(wxDatePickerCtrlAdapter, GetDatePickerCtrlAdapter)
	PROP_I(wxControl)
END_ADAPTER()

LWL_BEGIN_AGGREGATE_NO_COPY(wxCalendarCtrl)
	PROP_CO(wxCalendarCtrlAdapter, GetCalendarCtrlAdapter)
	PROP_I(wxControl)
LWL_END_AGGREGATE()

wxString htmlWinGetPage(const wxHtmlWindow &w)
{
    return wxString();
}

void htmlWinSetPage(wxHtmlWindow &w, const wxString &newValue)
{
    w.SetPage(newValue);
}

BEGIN_ADAPTER_NO_COPY(wxHtmlWindow)
    PROP_I(wxWindow) // actually wxScrolledWindow
    PROP_RW_EXT(wxString, htmlWinGetPage, htmlWinSetPage, "Page")
END_ADAPTER()

#endif  // DOXYGEN_INVOKED

namespace litwindow {
#if 0
    //-----------------------------------------------------------------------------------------------------------//
    //-----------------------------------------------------------------------------------------------------------//
    /// Default event type map. Maps wxWidget property names to event types so that changes to these properties
    /// can be monitored by the default ChangeMonitor.
    RapidUI::ChangeMonitorEventTypeMap &RapidUI::GetChangeMonitorEventTypeMap() const
    {
        static map<string, wxEventType> eventTypeMap;
        static bool _init=false;
        if (!_init) {
			RapidUI::PropertyEventType defaultPropertyEventTypes[]={
				{"wxRadioBox.Value", wxEVT_COMMAND_RADIOBOX_SELECTED},
                {"wxSpinCtrl.Value", wxEVT_COMMAND_SPINCTRL_UPDATED},
                {"wxChoice.Value", wxEVT_COMMAND_CHOICE_SELECTED}
			};
            size_t i;
            for (i=0; i<WXSIZEOF(defaultPropertyEventTypes); ++i) {
				pair<string, wxEventType> newValue=make_pair(defaultPropertyEventTypes[i].name, defaultPropertyEventTypes[i].type);
                eventTypeMap.insert(newValue);
			}
        }
        return eventTypeMap;
    }
#endif

void lwAdapterBase::OnWindowDestroy(wxWindowDestroyEvent &evt)
{
    wxWindow *w=evt.GetWindow();
    w->RemoveEventHandler(this);
    delete this;
}

};

BEGIN_EVENT_TABLE(lwAdapterBase, wxEvtHandler)
EVT_WINDOW_DESTROY(OnWindowDestroy)
END_EVENT_TABLE()

IMPLEMENT_ABSTRACT_CLASS(lwAdapterBase, wxEvtHandler);

BEGIN_ADAPTER_ABSTRACT(lwAdapterBase)
END_ADAPTER()
