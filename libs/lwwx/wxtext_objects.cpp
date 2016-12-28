/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: wxtext_objects.cpp,v 1.10 2007/12/11 11:28:15 Merry\Hajo Kirchhoff Exp $
*/
#include "stdwx.h"
#include "wxtext_objects.h"
#include "litwindow/dataadapter.h"
#include "litwindow/wx/rapidui.h"
#include "litwindow/renderer.hpp"
#include <sstream>
#include <limits>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace litwindow {

	void wxTextCtrlAdapter::OnCommitPhase(wxCommandEvent &evt)
	{
		if (wxWindow::FindFocus()==m_wnd)
			CopyFromWidget();
	}

	double wxTextCtrlAdapter::GetDouble() const
	{
		if (get_value_internal().empty())
			m_asDouble=numeric_limits<double>::infinity();
		else {
			tistringstream s((const TCHAR*)get_value_internal().c_str());
			m_asDouble=0.0;
			s >> m_asDouble;
		}
		return m_asDouble;
	}

	void wxTextCtrlAdapter::SetDouble(double d)
	{
		m_activeType=t_double;
		m_asDouble=d;
		if (!is_type<double>(m_value))
			ChangeValueAccessor(make_accessor(m_asDouble));
		if (m_asDouble==numeric_limits<double>::infinity())
			set_value_internal(wxEmptyString);
		else {
			wxString format=GetFormatOrDefault(wxT("%lf"));
			wxString value=wxString::Format(format, d);
			set_value_internal(value);
		}
		GetDouble();
	}

	void wxTextCtrlAdapter::set_value_internal(const wxString &new_value)
	{
		m_asString=new_value;
		if (m_is_null==false) {
			++m_disable_on_text;
			m_wnd->SetValue(new_value);
			--m_disable_on_text;
		}
	}

	wxString wxTextCtrlAdapter::get_value_internal() const
	{
		if (m_is_null==false) {
			m_asString=m_wnd->GetValue();
		}
		return m_asString;
	}

	void wxTextCtrlAdapter::OnText(wxCommandEvent &evt)
	{
		if (m_disable_on_text==0 && m_is_nullable) {
			wxString newText=m_wnd->GetValue();
			if (newText.empty()==false && m_is_null)
				m_asString=newText;	// NULL -> non-NULL transition clears current hidden text
			// set null, but only if SetValue did not start out with an empty string
			SetNull(newText.empty() && m_null_is_empty_string);
		}
		evt.Skip();
	}

	wxTextCtrlAdapter::wxTextCtrlAdapter(wxTextCtrl *w)
		:m_wnd(w)
		,m_activeType(t_text)
		,m_is_null(false)
		,m_is_nullable(false)
		,m_disable_on_text(0)
		,m_null_is_empty_string(true)
	{
		// it may be tempting to assing m_value=GetWndAggregate()["Label"] here.
		// DONT! GetWndAggregate() requires that the coobject has already been created!!!
		// which isn't the case as we are in the constructor of this coobject!
	}

	void wxTextCtrlAdapter::CopyFromWidget()
	{
		if (m_value.is_valid()) {
			SetNull(m_wnd->GetValue().empty() && m_null_is_empty_string);
			if (is_type<double>(m_value)) {
				SetDouble(GetDouble());
				NotifyChanged(GetWndAggregate()["Value"], false, false);
				NotifyChanged(GetWndAggregate()["Double"], false);
			} else if (is_type<wxString>(m_value)) {
				NotifyChanged(GetWndAggregate()["Value"], false, false);
				NotifyChanged(GetWndAggregate()["Label"], false);
			} else {
				wxLogError(wxT("Unknown type assigned to the wxTextCtrl control! Value will not be updated!"));
			}
		}
	}

	void wxTextCtrlAdapter::OnKillFocus(wxFocusEvent &evt)
	{
		CopyFromWidget();
		evt.Skip();
	}

	accessor wxTextCtrlAdapter::GetValue() const
	{
		if (m_is_null==false) {
			m_asString=m_wnd->GetValue();
		}
		if (!m_value.is_valid())
			m_value=make_accessor(m_asString);
		return m_value;
	}

	void wxTextCtrlAdapter::ChangeValueAccessor(const accessor &newValue)
	{
		if (!m_value.is_alias_of(newValue)) {
			// mark current m_value as invalid
			NotifyChanged(GetWndAggregate()["Value"], false, false);
			// assign new accessor
			m_value=newValue;
			// and send change notification for new accessor
			NotifyChanged(GetWndAggregate()["Value"], false);
		}
	}

	void wxTextCtrlAdapter::SetValue(const accessor &newValue)
	{
		if (is_type<double>(newValue)) {
			typed_accessor<double> doubleValue(dynamic_cast_accessor<double>(newValue));
			doubleValue.get(m_asDouble);
			SetDouble(m_asDouble);
			ChangeValueAccessor(make_accessor(m_asDouble));
		} else {
			litwindow::renderer<tstring> &r(litwindow::renderer<tstring>::get());
			wxString theValue=r(newValue, (const TCHAR*)GetFormat().c_str());
			// if string is empty, then empty strings will NOT count as NULL
			m_null_is_empty_string=GetNull() || theValue.empty()==false;
			set_value_internal(theValue);
			ChangeValueAccessor(make_accessor(m_asString));
		}
	}

	void wxTextCtrlAdapter::SetNull(bool is_null)
	{
		if (m_is_nullable==false)
			is_null=false;
		if (m_is_null!=is_null) {
			m_is_null=is_null;
			++m_disable_on_text;
			m_wnd->SetValue(m_is_null ? wxEmptyString : m_asString);
			--m_disable_on_text;
			m_wnd->SetBackgroundColour(m_is_null ? wxColour(237, 255, 255) : wxColour(255, 255, 255));
			m_wnd->SetForegroundColour(m_is_null ? m_wnd->GetBackgroundColour() : wxColour(0,0,0));
			m_wnd->Refresh();
			NotifyChanged(*this, "Null");
		}
	}

	void wxTextCtrlAdapter::SetNullable(bool is_nullable)
	{
		if (m_is_nullable!=is_nullable) {
			m_is_nullable=is_nullable;
			NotifyChanged(*this, "Nullable");
		}
	}

	/*===========================================================================================*/
	/*===========================================================================================*/

	void wxStaticTextAdapter::SetValue(const accessor &newValue)
	{
		litwindow::renderer<tstring> &r(litwindow::renderer<tstring>::get());
		m_text=r(newValue, (const TCHAR*)GetFormat().c_str());
		if (m_text!=m_wnd->GetLabel()) {
			m_wnd->SetLabel(m_text);
			litwindow::NotifyChanged(*this, "Value");/* , false, false);
						litwindow::NotifyChanged(*this, "Label", false, true);*/
			
		}
	}
	accessor wxStaticTextAdapter::GetValue() const
	{
		return myaccessor;
	}

	wxStaticTextAdapter::wxStaticTextAdapter(wxStaticText *w/* =0 */)
		:m_wnd(w)
	{
		myaccessor=make_accessor(m_text);
	}

	const wxString &lwTextAdapterBase::GetFormat() const
	{
		if (m_format.IsEmpty()) {
			m_format=GetWindowFormatString(GetWnd());
		}
		return m_format;
	}

	//-----------------------------------------------------------------------------------------------------------//
	//-----------------------------------------------------------------------------------------------------------//
	void wxSpinCtrlAdapter::OnKillFocus(wxFocusEvent &evt)
	{
		NotifyChanged(GetWndAggregate()["Value"], false);
		evt.Skip();
	}

	void wxSpinCtrlAdapter::OnSpinCtrl(wxSpinEvent &evt)
	{
		NotifyChanged(GetWndAggregate()["Value"], false);
		evt.Skip();
	}

};

using namespace litwindow;

IMPLEMENT_ABSTRACT_CLASS(lwTextAdapterBase, lwAdapterBase)
BEGIN_ADAPTER_ABSTRACT(lwTextAdapterBase)
PROP_I(lwAdapterBase)
PROP_GetSet(wxString, Format)
END_ADAPTER()

///@todo move "Double", "Type" and "Value" to lwTextAdapterBase so that wxSpinCtrlAdapter can share the code

BEGIN_ADAPTER_NO_COPY(wxTextCtrlAdapter)
PROP_I(lwTextAdapterBase)
PROP_GetSet(double, Double)
PROP_GetSet(int, Type)
PROP_GetSet(accessor, Value)
PROP_GetSet(bool, Null)
PROP_GetSet(bool, Nullable)
END_ADAPTER()

IMPLEMENT_DYNAMIC_CLASS(wxStaticTextAdapter, lwTextAdapterBase)
LWL_BEGIN_AGGREGATE_NO_COPY(wxStaticTextAdapter)
PROP_I(lwTextAdapterBase)
PROP_GetSet(accessor, Value)
LWL_END_AGGREGATE()

IMPLEMENT_DYNAMIC_CLASS(wxTextCtrlAdapter, lwTextAdapterBase)
BEGIN_EVENT_TABLE(wxTextCtrlAdapter, lwTextAdapterBase)
EVT_COMMAND(wxID_ANY, lwEVT_COMMIT_PHASE, OnCommitPhase)
EVT_TEXT(wxID_ANY, OnText)
EVT_KILL_FOCUS(OnKillFocus)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(wxSpinCtrlAdapter, lwTextAdapterBase)
BEGIN_EVENT_TABLE(wxSpinCtrlAdapter, lwTextAdapterBase)
EVT_KILL_FOCUS(OnKillFocus)
EVT_SPINCTRL(wxID_ANY, OnSpinCtrl)
END_EVENT_TABLE()

BEGIN_ADAPTER_NO_COPY(wxSpinCtrlAdapter)
PROP_I(lwTextAdapterBase)
PROP_GetSet(int, Value)
END_ADAPTER()
