/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: wxbutton_objects.h,v 1.4 2007/08/29 15:17:38 Hajo Kirchhoff Exp $
*/
#ifndef _LW_WXBUTTONOBJECTS_
#define _LW_WXBUTTONOBJECTS_

#pragma once

#include <litwindow/lwbase.hpp>
#include <litwindow/dataadapter.h>
#include <wx/checkbox.h>
#include "./base_objects.h"
#include "litwindow/wx/action_objects.h"

#ifndef x_DOC_DEVELOPER

namespace litwindow {

	class wxCheckBoxAdapter:public lwAdapterBase
	{
		wxCheckBox  *m_wnd;
		bool m_value;	///< supplemental value member while IsNull()
	public:
		wxCheckBoxAdapter(wxCheckBox *w=0)
			:m_wnd(w), m_value(false)
		{
		}
		wxWindow *GetWnd() const { return m_wnd; }
		bool    GetValue() const
		{
			return GetNull() ? m_value : m_wnd->GetValue();
		}
		void    SetValue(bool newValue)
		{
			m_value=newValue;
			if (GetNull()==false)
				m_wnd->SetValue(newValue);
		}
		void	OnCheckBox(wxCommandEvent &evt);
		bool	GetNull() const;
		void	SetNull(bool  newNull);
		bool	GetNullable() const;
		void	SetNullable(bool newNullable);
		DECLARE_DYNAMIC_CLASS_NO_COPY(wxCheckBoxAdapter)
		DECLARE_EVENT_TABLE()
	};

	inline wxCheckBoxAdapter *GetCheckBoxAdapter(wxCheckBox *w) { return GetAdapter<wxCheckBoxAdapter>(w, CLASSINFO(wxCheckBoxAdapter)); }

	class wxRadioBoxAdapter:public lwAdapterBase
	{
		wxRadioBox *m_wnd;
	public:
		wxRadioBoxAdapter(wxRadioBox *w=0)
			:m_wnd(w)
		{
		}
		wxWindow *GetWnd() const { return m_wnd; }
		int GetValue() const
		{
			return m_wnd->GetSelection();
		}
		void SetValue(int newValue)
		{
			m_wnd->SetSelection(newValue);
		}
		void OnRadioBox(wxCommandEvent &evt);
		DECLARE_DYNAMIC_CLASS_NO_COPY(wxRadioBoxAdapter)
		DECLARE_EVENT_TABLE()
	};

	inline wxRadioBoxAdapter *GetRadioBoxAdapter(wxRadioBox *w) { return GetAdapter<wxRadioBoxAdapter>(w, CLASSINFO(wxRadioBoxAdapter)); }

	class wxRadioButtonAdapter:public lwAdapterBase
	{
		wxRadioButton *m_wnd;
	public:
		wxRadioButtonAdapter(wxRadioButton *w=0)
			:m_wnd(w)
		{
		}
		wxWindow *GetWnd() const { return m_wnd; }
		bool GetValue() const
		{
			return m_wnd->GetValue();
		}
		void SetValue(bool newValue)
		{
			m_wnd->SetValue(newValue);
		}
		void OnRadioButton(wxCommandEvent &evt);
		DECLARE_DYNAMIC_CLASS_NO_COPY(wxRadioButtonAdapter)
		DECLARE_EVENT_TABLE()
	};

	inline wxRadioButtonAdapter *GetRadioButtonAdapter(wxRadioButton *w) { return GetAdapter<wxRadioButtonAdapter>(w, CLASSINFO(wxRadioButtonAdapter)); }

	class wxButtonAdapter:public lwAdapterBase
	{
		wxButton *m_wnd;
		bool	m_is_pressed;
	public:
		wxButtonAdapter(wxButton *w=0)
			:m_wnd(w),m_is_pressed(false)
		{
		}
		wxWindow *GetWnd() const { return m_wnd; }
		bool GetValue() const
		{
			return m_is_pressed;
		}
		void SetValue(bool new_value);
		void OnButtonPressed(wxCommandEvent &evt);
		DECLARE_DYNAMIC_CLASS_NO_COPY(wxButtonAdapter)
		DECLARE_EVENT_TABLE()
	};
	inline wxButtonAdapter *GetButtonAdapter(wxButton *w) { return GetAdapter<wxButtonAdapter>(w, CLASSINFO(wxButtonAdapter)); }

};

#endif

#endif

