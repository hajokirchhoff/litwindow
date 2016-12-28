/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: wxmisc_objects.h,v 1.3 2007/04/12 08:36:52 Hajo Kirchhoff Exp $
*/
#ifndef _LW_WXMISC_OBJECTS_
#define _LW_WXMISC_OBJECTS_

#pragma once

#include <litwindow/lwbase.hpp>
#include <litwindow/dataadapter.h>
#include <wx/datetime.h>
#include <wx/datectrl.h>
#include "./base_objects.h"

#ifndef x_DOC_DEVELOPER

class wxDatePickerCtrl;
class wxDateEvent;
class wxCalendarCtrl;
class wxCalendarEvent;

namespace litwindow {

class wxDatePickerCtrlAdapter:public lwAdapterBase
{
	wxDatePickerCtrl *m_wnd;
	void OnDateChanged(wxDateEvent &evt);
	void notify_changed() const;
public:
	wxDatePickerCtrlAdapter(wxDatePickerCtrl *w=0)
		:m_wnd(w)
	{
	}

	wxWindow *GetWnd() const { return m_wnd; }
	wxDateTime GetValue() const
	{
		return m_wnd->GetValue();
	}
	void SetValue(const wxDateTime &newValue);
	bool GetIsValid() const { return GetValue().IsValid(); }
	void SetIsValid(bool valid) 
	{ 
		if (valid!=GetIsValid())  {
			SetValue(valid ? wxDateTime::Now() : wxDateTime());
		}
	}
	DECLARE_DYNAMIC_CLASS_NO_COPY(wxDatePickerCtrlAdapter)
	DECLARE_EVENT_TABLE()
};

class wxCalendarCtrlAdapter:public lwAdapterBase
{
public:
	wxCalendarCtrl *m_wnd;
	void OnDateChanged(wxCalendarEvent &evt);
	void notify_changed() const;
public:
	wxCalendarCtrlAdapter(wxCalendarCtrl*w=0)
		:m_wnd(w)
	{
	}

	wxCalendarCtrl *GetWnd() const { return m_wnd; }
	wxDateTime GetValue() const;
	void SetValue(const wxDateTime &newValue);
	bool GetIsValid() const { return GetValue().IsValid(); }
	void SetIsValid(bool valid) 
	{ 
		if (valid!=GetIsValid())  {
			SetValue(valid ? wxDateTime::Now() : wxDateTime());
		}
	}
	DECLARE_DYNAMIC_CLASS_NO_COPY(wxCalendarCtrlAdapter)
	DECLARE_EVENT_TABLE()
};

inline wxDatePickerCtrlAdapter *GetDatePickerCtrlAdapter(wxDatePickerCtrl *w) { return GetAdapter<wxDatePickerCtrlAdapter>(w, CLASSINFO(wxDatePickerCtrlAdapter)); }
inline wxCalendarCtrlAdapter *GetCalendarCtrlAdapter(wxCalendarCtrl *w) { return GetAdapter<wxCalendarCtrlAdapter>(w, CLASSINFO(wxCalendarCtrlAdapter)); }

};

#endif

#endif

