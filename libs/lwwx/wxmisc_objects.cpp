/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: wxmisc_objects.cpp,v 1.3 2007/04/12 08:36:52 Hajo Kirchhoff Exp $
*/
#include "stdwx.h"
#include <wx/dateevt.h>
#include <wx/calctrl.h>
#include "wxmisc_objects.h"
#include "litwindow/dataadapter.h"
#include "litwindow/wx/rapidui.h"
#include <sstream>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace litwindow {

void wxDatePickerCtrlAdapter::OnDateChanged(wxDateEvent &evt)
{
	notify_changed();
	evt.Skip();
}

void wxCalendarCtrlAdapter::OnDateChanged(wxCalendarEvent &evt)
{
	notify_changed();
	evt.Skip();
}

void wxDatePickerCtrlAdapter::SetValue(const wxDateTime &newValue)
{
	wxDateTime current(m_wnd->GetValue());
	if (current.IsValid()!=newValue.IsValid() || newValue.IsValid() && newValue!=m_wnd->GetValue()) {
		m_wnd->SetValue(newValue);
		notify_changed();
	}
}

void wxCalendarCtrlAdapter::SetValue(const wxDateTime &newValue)
{
	wxDateTime current(m_wnd->GetDate());
	if (current.IsValid()!=newValue.IsValid() || newValue.IsValid() && newValue!=current) {
		m_wnd->SetDate(newValue);
		notify_changed();
	}
}

wxDateTime wxCalendarCtrlAdapter::GetValue() const
{
	return m_wnd->GetDate();
}


void wxDatePickerCtrlAdapter::notify_changed() const
{
	RapidUI::NotifyChanged(make_aggregate(*m_wnd)["Value"], false, false);
	RapidUI::NotifyChanged(make_aggregate(*m_wnd)["IsValid"], false);
}

void wxCalendarCtrlAdapter::notify_changed() const
{
	RapidUI::NotifyChanged(make_aggregate(*m_wnd)["Value"], false, false);
	RapidUI::NotifyChanged(make_aggregate(*m_wnd)["IsValid"], false);
}
};

using namespace litwindow;

IMPLEMENT_DYNAMIC_CLASS(wxDatePickerCtrlAdapter, lwAdapterBase)
BEGIN_EVENT_TABLE(wxDatePickerCtrlAdapter, lwAdapterBase)
EVT_DATE_CHANGED(wxID_ANY, OnDateChanged)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(wxCalendarCtrlAdapter, lwAdapterBase)
BEGIN_EVENT_TABLE(wxCalendarCtrlAdapter, lwAdapterBase)
EVT_CALENDAR_SEL_CHANGED(wxID_ANY, OnDateChanged)
END_EVENT_TABLE()

BEGIN_ADAPTER_NO_COPY(wxDatePickerCtrlAdapter)
PROP_I(lwAdapterBase)
PROP_GetSet(wxDateTime, Value)
PROP_GetSet(bool, IsValid)
END_ADAPTER()

LWL_BEGIN_AGGREGATE_NO_COPY(wxCalendarCtrlAdapter)
PROP_I(lwAdapterBase)
PROP_GetSet(wxDateTime, Value)
PROP_GetSet(bool, IsValid)
LWL_END_AGGREGATE()
