/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: wxtext_objects.h,v 1.5 2006/11/29 12:34:43 Hajo Kirchhoff Exp $
*/
#ifndef _LW_WXTEXTOBJECTS_
#define _LW_WXTEXTOBJECTS_
#pragma once

#include <litwindow/lwbase.hpp>
#include <litwindow/dataadapter.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include "./base_objects.h"

#ifndef x_DOC_DEVELOPER

namespace litwindow {

/** Base class for adapters for text related controls. */
class lwTextAdapterBase:public lwAdapterBase
{
	mutable wxString    m_format;
public:
	lwTextAdapterBase() {}
	virtual wxWindow *GetWnd() const = 0;
	virtual aggregate GetWndAggregate() const = 0;
	const wxString &GetFormat() const;
	wxString GetFormatOrDefault(const wxString &defaultFormat)
	{
		wxString rc=GetFormat();
		if (rc.IsEmpty())
			rc=defaultFormat;
		return rc;
	}
	void SetFormat(const wxString &fmt)
	{
		m_format=fmt;
	}
	DECLARE_ABSTRACT_CLASS(lwTextAdapterBase)
};

class wxStaticTextAdapter:public lwTextAdapterBase
{
	wxStaticText *m_wnd;
	wxString m_text;
	accessor myaccessor;
public:
	wxStaticTextAdapter(wxStaticText *w=0);
	wxWindow *GetWnd() const { return m_wnd; }
	aggregate GetWndAggregate() const { return make_aggregate(*m_wnd); }
	accessor GetValue() const;
	void SetValue(const accessor &newValue);
	DECLARE_DYNAMIC_CLASS_NO_COPY(wxStaticTextAdapter)
};

class wxTextCtrlAdapter:public lwTextAdapterBase
{
	wxTextCtrl *m_wnd;
	mutable double  m_asDouble;
	mutable wxString m_asString;
	enum {
		t_double,
		t_text
	}   m_activeType;
	int     m_inputType;
	bool m_is_null;
	bool m_is_nullable;
	size_t m_disable_on_text;

	/// used to distinguish between an empty string and a NULL value
	/// when SetValue is passed an empty string while m_is_null==false,
	/// empty string will count as empty string, not NULL.
	/// when m_is_null is true, empty string will count as NULL.
	bool m_null_is_empty_string;

	mutable accessor    m_value;
protected:
	void ChangeValueAccessor(const accessor &newValue);
	void CopyFromWidget();
	void set_value_internal(const wxString &new_value);
	wxString get_value_internal() const;

public:
	wxTextCtrlAdapter(wxTextCtrl *w=0);
	wxWindow *GetWnd() const { return m_wnd; }
	aggregate GetWndAggregate() const { return make_aggregate(*m_wnd); }
	double GetDouble() const;
	void SetDouble(double d);
	accessor GetValue() const;
	void SetValue(const accessor &newValue);

	bool GetNull() const { return m_is_null; }
	void SetNull(bool is_null);

	bool GetNullable() const { return m_is_nullable; }
	void SetNullable(bool is_nullable);

	int GetType() const
	{
		return m_inputType;
	}
	void SetType(int newType)
	{
	}

	void OnText(wxCommandEvent &evt);
	void OnKillFocus(wxFocusEvent &evt);
	void OnCommitPhase(wxCommandEvent &evt);
	DECLARE_DYNAMIC_CLASS_NO_COPY(wxTextCtrlAdapter)
	DECLARE_EVENT_TABLE()
};

class wxSpinCtrlAdapter:public lwTextAdapterBase
{
	wxSpinCtrl *m_wnd;
public:
	wxSpinCtrlAdapter(wxSpinCtrl *l=0)
		:m_wnd(l)
	{
	}
	wxWindow *GetWnd() const { return m_wnd; }
	aggregate GetWndAggregate() const { return make_aggregate(*m_wnd); }

	void OnKillFocus(wxFocusEvent &evt);
	void OnSpinCtrl(wxSpinEvent &evt);
	int GetValue() const
	{
		return m_wnd->GetValue();
	}
	void SetValue(int newValue)
	{
		m_wnd->SetValue(newValue);
	}
	DECLARE_DYNAMIC_CLASS_NO_COPY(wxSpinCtrlAdapter)
	DECLARE_EVENT_TABLE()
};

inline wxTextCtrlAdapter *GetTextCtrlAdapter(wxTextCtrl *w) { return GetAdapter<wxTextCtrlAdapter>(w, CLASSINFO(wxTextCtrlAdapter)); }
inline wxSpinCtrlAdapter *GetSpinCtrlAdapter(wxSpinCtrl *w) { return GetAdapter<wxSpinCtrlAdapter>(w, CLASSINFO(wxSpinCtrlAdapter)); }
inline wxStaticTextAdapter *GetStaticTextAdapter(wxStaticText *w) { return GetAdapter<wxStaticTextAdapter>(w, CLASSINFO(wxStaticTextAdapter)); }
};

#endif

#endif
