/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: wxList_objects.cpp,v 1.4 2008/03/05 17:32:21 Merry\Hajo Kirchhoff Exp $
*/
#include "stdwx.h"
using namespace std;
#include <wx/event.h>
#include "litwindow/lwbase.hpp"
#include "litwindow/dataadapter.h"
#include <litwindow/dataadapterenum.h>
#include "litwindow/wx/rapidUI.h"
#include "wxlist_objects.h"

#define new DEBUG_NEW

namespace litwindow {

IMPLEMENT_ABSTRACT_CLASS(lwListAdapterBase, lwAdapterBase);
IMPLEMENT_DYNAMIC_CLASS(wxListBoxAdapter, lwListAdapterBase);
IMPLEMENT_DYNAMIC_CLASS(wxComboBoxAdapter, lwListAdapterBase);
IMPLEMENT_DYNAMIC_CLASS(wxChoiceAdapter, lwListAdapterBase);

void lwListAdapterBase::FillList()
{
	if (!m_items.is_valid()) {
		ClearList();
		GetWnd()->Disable();
	} else if (m_items.is_container()) {
		bool logWarningShown=false;
		tstring currentSelection=GetStringSelection();
		ClearList();
		GetWnd()->Enable();
		const_container c(m_items.get_container());
		const_container::const_iterator i;
		tstring newValue;
		int count=0;
		int foundSelection=-1;
		for (i=c.begin(); i!=c.end(); ++i) {
			const_accessor element(*i);
			if (element.is_aggregate()) {
				const_aggregate ag=element.get_aggregate();
				const_aggregate::const_iterator displayElement;
				if (m_column.length()==0)
					displayElement=ag.begin();
				else
					displayElement=ag.find(t2string(m_column));
				if (displayElement!=ag.end())
					newValue=displayElement->to_string();
				else {
					if (!logWarningShown) {
						wxLogWarning(wxT("Column %s does not exist in container element %s"), m_column.c_str(), accessor_as_debug(*displayElement));
						logWarningShown=true;
					}
					newValue=wxT("- ? -");
				}
			} else if (!element.is_container())
				newValue=element.to_string().c_str();
			AppendList(newValue);
			if (newValue==currentSelection) {
				foundSelection=count;
			}
			++count;
		}
		if (foundSelection>=0)
			SetSelection(foundSelection);
	}
}

long lwListAdapterBase::GetClientDataSelection() const
{
	int index=GetSelection();
	return index!=-1 ? (long)GetClientData(index) : -1;
}

void lwListAdapterBase::SetClientDataSelection(long clientData)
{
	int i=GetCount()-1;
	while (i>=0 && (long)GetClientData(i)!=clientData)
		--i;
	SetSelection(i);
}

int lwListAdapterBase::GetValueAsInt() const
{
	return IndexClientData ? GetClientDataSelection() : GetSelection();
}
void lwListAdapterBase::SetValueAsInt(int newValue)
{
	if (IndexClientData)
		SetClientDataSelection(newValue);
	else
		SetSelection(newValue);
}

const accessor lwListAdapterBase::GetCurrent() const
{
	if (!m_current.is_valid() && m_items.is_valid()) {
		int currentSelection=GetSelection();
		CalcCurrent(currentSelection);
	}
	return m_current;
}

void lwListAdapterBase::CalcCurrent(int containerIndex) const
{
	if (m_items.is_valid() && m_items.is_container()) {
		// the const_accessor points to the end item which is invalid
		// but the type of the item is valid.
		container c=m_items.get_container();
		container::iterator i=containerIndex>=0 ? m_items.get_container().at(containerIndex) : m_items.get_container().end();
		//accessor the_item=*i;
		prop_t type=m_items.get_container().get_element_type();
		if (!m_current.is_valid())
			m_current=create_object(type);
		if (i!=c.end())
			m_current.from_accessor(*i/*the_item*/);
		else {
			accessor empty=create_object(type);
			try {
				m_current.from_accessor(empty);
			}
			catch (...) {
				empty.destroy();
				throw;
			}
			empty.destroy();
		}
		NotifyChanged(m_current, true);
	}
}

void lwListAdapterBase::SetItems(const accessor &newValue)
{
	m_items=newValue;
	FillList();
	if (m_current.is_valid()) {
		NotifyChanged(m_current, true, false);
		m_current.destroy();
	}
	CalcCurrent(-1);
}

void lwListAdapterBase::SetCurrent(const accessor &newValue)
{
}

lwListAdapterBase::~lwListAdapterBase()
{
	m_current.destroy();
}

const accessor lwListAdapterBase::GetValue() const
{
    accessor rc;
    if (is_enum()) {
        m_int_value=GetClientDataSelection();
        rc=reinterpret_accessor(m_enum_type, make_accessor(m_int_value));
    } else {
        m_int_value=GetValueAsInt();
        rc=make_accessor(m_int_value);
    }
    return rc;
}

void lwListAdapterBase::SetValue(const accessor &newValue)
{
    if (newValue.is_enum()) {
        set_is_enum(true);
        m_enum_type=newValue.get_type();
        const converter_enum_info *new_info=newValue.get_enum_info();
        if (new_info!=m_enum_info) {
            m_enum_info=new_info;
            ClearList();
            for (size_t i=0; i<m_enum_info->enum_count(); ++i) {
                const converter_enum_info::element &current(m_enum_info->enum_value(i));
                AppendList(current.m_name, (void*)current.m_value);
            }
            SetUseClientData(true);
        }
        SetClientDataSelection(newValue.to_int());
    } else {
        set_is_enum(false);
        if (m_enum_info) {
            SetUseClientData(false);
            m_enum_info=0;
        }
        SetValueAsInt(newValue.to_int());
    }
}

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//

void wxListBoxAdapter::OnListBox(wxCommandEvent &evt)
{
	CalcCurrent(m_wnd->GetSelection());
	aggregate a(make_aggregate(*m_wnd));
	NotifyChanged(a["StringSelection"], false, false);
	NotifyChanged(a["Value"], false);
	evt.Skip();
}


//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//

void wxComboBoxAdapter::OnComboBox(wxCommandEvent &evt)
{
	aggregate a(make_aggregate(*m_wnd));
	CalcCurrent(m_wnd->GetSelection());
	NotifyChanged(a["StringSelection"], false, false);
	NotifyChanged(a["Value"], false);
	evt.Skip();
}

void wxComboBoxAdapter::OnKillFocus(wxFocusEvent &evt)
{
	aggregate a(make_aggregate(*m_wnd));
	NotifyChanged(a["StringSelection"], false, false);
	NotifyChanged(a["Value"], false);
	evt.Skip();
}

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
void wxChoiceAdapter::OnChoice(wxCommandEvent &evt)
{
	CalcCurrent(m_wnd->GetSelection());
	aggregate a(make_aggregate(*m_wnd));
	NotifyChanged(a["StringSelection"], false, false);
	NotifyChanged(make_aggregate(*m_wnd)["Value"], false);
	evt.Skip();
}

};

using namespace litwindow;

BEGIN_EVENT_TABLE(wxListBoxAdapter, lwListAdapterBase)
EVT_LISTBOX(wxID_ANY, OnListBox)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(wxComboBoxAdapter, lwListAdapterBase)
EVT_COMBOBOX(wxID_ANY, OnComboBox)
EVT_KILL_FOCUS(OnKillFocus)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(wxChoiceAdapter, lwListAdapterBase)
EVT_CHOICE(wxID_ANY, OnChoice)
END_EVENT_TABLE()

/**@todo implement a BEGIN_ADAPTER_ABSTRACT, so that "Items", "Columns" and "Current"
properties can move to a BEGIN_ADAPTER_ABSTRACT(lwListAdapterBase) where they belong. */

BEGIN_ADAPTER_ABSTRACT(lwListAdapterBase)
PROP_I(lwAdapterBase)
PROP_GetSet(accessor, Items)
PROP_GetSet(tstring, Column)
PROP_GetSet(accessor, Current)
PROP_GetSet(wxString, StringSelection)
PROP_GetSet(int, ValueAsInt)
PROP_GetSet(accessor, Value)
PROP_GetSet(bool, UseClientData)
PROP(IndexClientData)
END_ADAPTER()

BEGIN_ADAPTER_NO_COPY(wxListBoxAdapter)
PROP_I(lwListAdapterBase)
END_ADAPTER()

BEGIN_ADAPTER_NO_COPY(wxComboBoxAdapter)
PROP_I(lwListAdapterBase)
END_ADAPTER()

BEGIN_ADAPTER_NO_COPY(wxChoiceAdapter)
PROP_I(lwListAdapterBase)
END_ADAPTER()
