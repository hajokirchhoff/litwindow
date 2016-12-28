/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: wxbutton_objects.cpp,v 1.5 2007/12/11 11:28:15 Merry\Hajo Kirchhoff Exp $
*/
#include "stdwx.h"
#include "wxbutton_objects.h"
#include "litwindow/dataadapter.h"
#include "litwindow/wx/rapidui.h"
#include <sstream>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace litwindow {

void wxCheckBoxAdapter::OnCheckBox(wxCommandEvent &evt)
{
    NotifyChanged(make_aggregate(*m_wnd)["Value"], false);
    evt.Skip();
}

void wxRadioBoxAdapter::OnRadioBox(wxCommandEvent &evt)
{
    NotifyChanged(make_aggregate(*m_wnd)["Value"], false);
    evt.Skip();
}

void wxRadioButtonAdapter::OnRadioButton(wxCommandEvent &evt)
{
    NotifyChanged(make_aggregate(*m_wnd)["Value"], false);
    evt.Skip();
}

void wxButtonAdapter::OnButtonPressed(wxCommandEvent &evt)
{
	SetValue(true);	// toggle pressed down/up
	SetValue(false);
}

void wxButtonAdapter::SetValue(bool new_value)
{
	if (m_is_pressed!=new_value) {
		m_is_pressed=new_value;
		NotifyChanged(*this, "Value", true);	// must solve immediately. this is an action
	}
}

bool wxCheckBoxAdapter::GetNull() const
{
	return m_wnd->Is3State() && m_wnd->Get3StateValue()==wxCHK_UNDETERMINED;
}

void wxCheckBoxAdapter::SetNull(bool newNull)
{
	if (newNull!=GetNull()) {
		if (newNull)
			m_value=m_wnd->GetValue();
		// change the value of the checkbox only if NULL is requested   or     NON-NULL is requested and the current state is NULL
		if (newNull || newNull==false && m_wnd->Get3StateValue()==wxCHK_UNDETERMINED)
			m_wnd->Set3StateValue(newNull ? wxCHK_UNDETERMINED : (m_value ? wxCHK_CHECKED : wxCHK_UNCHECKED));
	}
}
bool wxCheckBoxAdapter::GetNullable() const
{
	return m_wnd->Is3State();
}
void wxCheckBoxAdapter::SetNullable(bool newNullable)
{
	long current_style=m_wnd->GetWindowStyle();
	if (newNullable) {
		current_style|=wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER;
	} else {
		current_style&=~(wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER);
	}
	m_wnd->SetWindowStyle(current_style);
}


};

using namespace litwindow;

IMPLEMENT_DYNAMIC_CLASS(wxCheckBoxAdapter, lwAdapterBase)
BEGIN_EVENT_TABLE(wxCheckBoxAdapter, lwAdapterBase)
EVT_CHECKBOX(wxID_ANY, OnCheckBox)
END_EVENT_TABLE()

BEGIN_ADAPTER_NO_COPY(wxCheckBoxAdapter)
PROP_I(lwAdapterBase)
PROP_GetSet(bool, Value)
PROP_GetSet(bool, Null)
PROP_GetSet(bool, Nullable)
END_ADAPTER()

IMPLEMENT_DYNAMIC_CLASS(wxRadioBoxAdapter, lwAdapterBase)
BEGIN_EVENT_TABLE(wxRadioBoxAdapter, lwAdapterBase)
EVT_RADIOBOX(wxID_ANY, OnRadioBox)
END_EVENT_TABLE()

BEGIN_ADAPTER_NO_COPY(wxRadioBoxAdapter)
PROP_I(lwAdapterBase)
PROP_GetSet(int, Value)
END_ADAPTER()

IMPLEMENT_DYNAMIC_CLASS(wxRadioButtonAdapter, lwAdapterBase)
BEGIN_EVENT_TABLE(wxRadioButtonAdapter, lwAdapterBase)
EVT_RADIOBUTTON(wxID_ANY, OnRadioButton)
END_EVENT_TABLE()

BEGIN_ADAPTER_NO_COPY(wxRadioButtonAdapter)
PROP_I(lwAdapterBase)
PROP_GetSet(bool, Value)
END_ADAPTER()

IMPLEMENT_DYNAMIC_CLASS(wxButtonAdapter, lwAdapterBase)
BEGIN_EVENT_TABLE(wxButtonAdapter, lwAdapterBase)
EVT_BUTTON(wxID_ANY, OnButtonPressed)
END_EVENT_TABLE()

LWL_BEGIN_AGGREGATE_NO_COPY(wxButtonAdapter)
PROP_I(lwAdapterBase)
PROP_GetSet(bool, Value)
LWL_END_AGGREGATE()
