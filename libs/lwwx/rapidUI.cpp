/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: rapidUI.cpp,v 1.10 2007/12/11 11:28:15 Merry\Hajo Kirchhoff Exp $
*/
#include "Stdwx.h"
#include "litwindow/wx/rapidUI.h"
#include "litwindow/algorithm.h"
#include "litwindow/logging.h"
#include "litwindow/check.hpp"
#include <wx/radiobox.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define DO_LOG_RAPIDUI

#ifdef DO_LOG_RAPDUI
#define wxLogRapidUI(a) wxLogDebug a
#else
#define wxLogRapidUI(a)
#endif

//DEFINE_EVENT_TYPE(lwEVT_COMMIT_PHASE);
LWWX_API const wxEventType lwEVT_COMMIT_PHASE = wxNewEventType();

using namespace std;

namespace litwindow {

RapidUI::RapidUISet_t RapidUI::g_rapidUISet;

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//

void RapidUI::Start()
{
	Pause();
	RulesList::const_iterator i;
	for (i=m_rulesList.begin(); i!=m_rulesList.end(); ++i)
		(*i)(*this);
	if (!m_defaultRulesAdded)
		AddDefaultRules();
	m_solver.execute_all_immediate();
	Resume();
}

void RapidUI::Apply(CommitPhase type)
{
	wxCommandEvent evt(lwEVT_COMMIT_PHASE);
	evt.SetInt(type);
	evt.StopPropagation();
	//wxTextCtrl *w=wxDynamicCast(wxWindow::FindFocus(), wxTextCtrl);
	wxWindow *w=wxWindow::FindFocus();
	if (w)
		w->GetEventHandler()->ProcessEvent(evt);
	//WindowList::iterator i;
	//for (i=m_widgets_in_a_rule.begin(); i!=m_widgets_in_a_rule.end(); ++i) {
	//	(*i)->ProcessEvent(evt);
	//}
}

void RapidUI::TransferDataFromWindow()
{
	UnsolveAllWindowsToDataRules();
	m_solver.solve();
}

void RapidUI::TransferDataToWindow()
{
    UnsolveAllDataToWindowsRules();
    m_solver.solve();
}

void RapidUI::UnsolveAllWindowsToDataRules()
{
	m_solver.mark_group_changed(wxT("window_to_data"));
}

void RapidUI::UnsolveAllDataToWindowsRules()
{
    m_solver.mark_group_changed(wxT("data_to_window"));
}

void RapidUI::AddDefaultRulesForWindow(wxWindow *w)
{
	if (w) {
		tstring windowName=GetWindowName(w);
		if (windowName==_T("LITWINDOW_RULES")) {
			// this is a 'specially' reserved window name. it should be a wxTextCtrl containing window rules.
			tstring rules;
			wxStaticText *the_rules=dynamic_cast<wxStaticText*>(w);
			if (the_rules)
				rules=the_rules->GetLabel();
			else {
				wxListBox *lb=dynamic_cast<wxListBox*>(w);
				wxTextCtrl *tc=dynamic_cast<wxTextCtrl*>(w);
				if (lb) {
					wxArrayString value(lb->GetStrings());
					for (size_t i=0; i<value.GetCount(); ++i)
						rules+=value[i]+_T("\n");
				} else if (tc) {
					rules=tc->GetValue();
				} else
					throw lwbase_error("Window with name LITWINDOW_RULES found, but it is not of type wxStaticText, wxListBox or wxTextCtrl");
			}
			AssignRules(rules, tstring(), _T("LITWINDOW_RULES"));
		} else {
			if (windowName.size()>0) {
				accessor theData=GetDataAccessor(windowName.c_str());
				if (theData.is_valid()) {
					//lw_log() << wxT("Default rule ") << windowName << wxT(" <-> ") << s2tstring(theData.name()) << endl;
					AssignTwoWay(w, theData);
					accessor winAccessor(GetWindowAccessor(w));
					if (winAccessor.is_valid() && winAccessor.is_aggregate()) {
						// now set all default properties for the window
						wxString windowProperties(GetWindowFormatProperties(w));
						if (windowProperties.length()>0) {
							aggregate ag(winAccessor.get_aggregate());
							map<tstring, tstring> properties=split_string(windowProperties.wc_str());
							for (map<tstring, tstring>::const_iterator i=properties.begin(); i!=properties.end(); ++i) {
								pair<aggregate::iterator, bool> member=ag.find_scope(t2string(i->first));
								if (member.second) {
									member.first->from_string(i->second);
								}
							}
						}
					}

				}/* else
					lw_log() << "no default for window " << windowName << endl;*/
			}
			wxWindowList &children=w->GetChildren();
			wxWindowList::compatibility_iterator child=children.GetFirst();
			while (child) {
				AddDefaultRulesForWindow(child->GetData());
				child=child->GetNext();
			}
		}
	}
}

void RapidUI::AssignTwoWay(const accessor &value, const accessor &data)
{
	rule_base *one=Assign(value, data, wxEmptyString);
	rule_base *two=Assign(data, value, wxEmptyString);
	one->set_mirror(two);
	two->set_mirror(one);
}

rule_base *RapidUI::Assign(const accessor &value, const const_accessor &data, const tstring &group)
{
	return Assign(new rule_assign(value, data), group);
}

rule_base *RapidUI::Assign(rule_base *r, const tstring &group)
{
	auto_ptr<rule_base> _r(r);
	if (!m_defaultRulesAdded)
		AddDefaultRules();
	m_solver.add_to_group(_r.release(), group);
	return r;
}

void RapidUI::AssignRules(const tstring &rules, const tstring &group, const tstring &from_source)
{
	if (!m_defaultRulesAdded)
		AddDefaultRules();
	add_rules(rules, group, from_source);
}

void RapidUI::AssignTwoWay(wxWindow *w, const accessor &data)
{
	// get an accessor for the default value and activate monitoring for that value
	accessor value(GetWindowDefaultValueAccessor(w));
	if (value.is_valid()) {
		rule_base *to_window=Assign(value, data, wxT("data_to_window"));
		rule_base *to_data=Assign(data, value, wxT("window_to_data"));
		to_data->set_mirror(to_window);
		to_window->set_mirror(to_data);
		m_widgets_in_a_rule.push_back(w);
	}
}

void RapidUI::AssignTwoWay(const tstring &windowAttributeName, const tstring &dataName)
{
	size_t delimiter=windowAttributeName.find('.');
	tstring windowName;
	tstring attributeName;
	if (delimiter==tstring::npos) {
		windowName=windowAttributeName;
		attributeName=_T("Value");
	} else {
		windowName=windowAttributeName.substr(0, delimiter);
		attributeName=windowAttributeName.substr(delimiter+1);
	}
	wxWindow *w=FindWindow(windowName);
	accessor theData=GetDataAccessor(dataName);
	if (w && theData.is_valid()) {
		accessor value(GetWindowAccessor(w, attributeName));
		AssignTwoWay(value, theData);
	}
}

accessor RapidUI::GetWindowAccessor(tstring name, bool addChangeMonitor)
{
	accessor rc;
	size_t delimiter=name.find('.');
	tstring attribute;
	if (delimiter==tstring::npos) {
		attribute=wxT("Value");
	} else {
		attribute=name.substr(delimiter+1);
		name.erase(delimiter);
	}
	wxWindow *w=FindWindow(name);
	if (w)
		rc=GetWindowAccessor(w, attribute);
	return rc;
}

accessor RapidUI::GetWindowAccessor(wxWindow *w)
{
	vector<wxString> untriedNames;
	size_t i=0;
	// do a width search up the class hierarchy to find a matching accessor
	wxString nextName(w->GetClassInfo()->GetClassName());
	untriedNames.push_back(nextName);
	prop_t type;
	do {
		//wxLogDebug(_T("Searching Window Accessor for class %s"), untriedNames[i].c_str());
		// try current class
		type=get_prop_type_by_name(t2string(untriedNames[i].wc_str()));
		if (type==0) {
			// no accessor found for current class, try base classes
			wxClassInfo *info=wxClassInfo::FindClass(untriedNames[i].c_str());
			if (info) {
				const wxChar *name=info->GetBaseClassName1();
				if (name)
					untriedNames.push_back(name);
				name=info->GetBaseClassName2();
				if (name)
					untriedNames.push_back(name);
			}
		}
	} while (type==0 && ++i<untriedNames.size());
	return type==0 ? accessor() : accessor(w, type);
}

accessor RapidUI::GetWindowAccessor(wxWindow *w, const tstring &attributeName)
{
	accessor rc=GetWindowAccessor(w);
	if (!rc.is_valid())
		return rc;
	if (rc.is_aggregate()) {
		aggregate ag=rc.get_aggregate();
		pair<aggregate::iterator, bool> i=ag.find_scope(t2string(attributeName));
		if (i.second==false)
			rc=accessor();
		else
			rc=*i.first;
	} else
		rc=accessor();
	wxLogRapidUI(("GetWindowAccessor %s.%s returns %s", GetWindowName(w).c_str(), attributeName.c_str(), accessor_as_debug(rc).c_str()));
	return rc;
}

accessor RapidUI::GetWindowDefaultValueAccessor(wxWindow *w)
{
	accessor value(GetWindowAccessor(w, wxT("Value")));
	if (!value.is_valid()) {
		lw_err() << "RapidUI handler for window " << GetWindowName(w) << " type " << w->GetClassInfo()->GetClassName() << " missing." << endl;
		//throw lwbase_error("no RapidUI handler for window found");
	}
	return value;
}

bool RapidUI::ValueChanged(const const_accessor &value, bool recursive, bool solve_immediately)
{
	bool rc=true;
	if (m_solver.is_enabled()) {
		static int nestingCount=0;
		// ignore nested 'value-changed' events that may occur as a result
		// of m_solver.solve()
		wxLogRapidUI(("ValueChanged( %s: %s )", value.name().c_str(), accessor_as_debug(value).c_str()));
		m_solver.mark_value_changed(value, recursive);
		if (++nestingCount==1 && solve_immediately) {
			litwindow::context_t c("RapidUI::ValueChanged - solving rules");
			try {
				m_solver.solve();
			}
			catch (constraint_solver::rules_conflict &c) {
				wxLogError(wxT("Validation failed for %s with error '%s'"), s2tstring(c.m_target.get_name()).c_str(), s2tstring(c.what()).c_str());
				rc=false;
			}
			catch (...) {
				--nestingCount;
				lw_err() << _T("unspecified exception during RapidUI::ValueChanged") << endl;
				throw;
			}
		}
		--nestingCount;
	}
	return rc;
}

void RapidUI::AddDefaultRules()
{
	m_defaultRulesAdded=true;
	WindowList::const_iterator i;
	for (i=m_windowList.begin(); i!=m_windowList.end(); ++i)
		AddDefaultRulesForWindow(*i);
}

void RapidUI::NotifyChanged(const const_accessor &value, bool recursive, bool solve_immediately)
{
	RapidUISet_t::iterator i;
	for (i=g_rapidUISet.begin(); i!=g_rapidUISet.end(); ++i) {
		(*i)->ValueChanged(value, recursive, solve_immediately);
	}
}

RapidUI::RapidUI(void)
:m_defaultRulesAdded(false)
,m_wndNamespace(wxT("wnd::"))
,m_dataNamespace(wxT("data::"))
{
	m_solver.set_symbol_table(this);
	g_rapidUISet.insert(this);
}

RapidUI::~RapidUI(void)
{
	g_rapidUISet.erase(this);
	Clear();
}

void RapidUI::Clear()
{
	m_windowList.clear();
	m_solver.clear();
	m_defaultRulesAdded=false;
	m_widgets_in_a_rule.clear();
	m_rulesList.clear();
	m_dataList.clear();
	m_dataName.clear();
}

void RapidUI::AddWindow(wxWindow *w)
{
	m_windowList.push_back(w);
}

void RapidUI::RemoveWindow(wxWindow *w)
{
	WindowList::iterator i=find(m_windowList.begin(), m_windowList.end(), w);
	if (i!=m_windowList.end()) {
		m_windowList.erase(i);
	}
	i=find(m_widgets_in_a_rule.begin(), m_widgets_in_a_rule.end(), w);
	if (i!=m_windowList.end())
		m_widgets_in_a_rule.erase(i);
}

tstring RapidUI::MakeUniqueName(const accessor &a) const
{
	ostringstream s;
	s << hex << (unsigned long*)a.get_this_ptr() << a.class_name();
	return s2tstring(s.str());
}

void RapidUI::AddData(const accessor &a, tstring name)
{
	if (name.size()==0) {
		name=MakeUniqueName(a);
	}
	m_dataList.push_back(a);
	m_dataName.push_back(name);
}

void RapidUI::AddRules(rules_function_t r)
{
	m_rulesList.push_back(r);
}

accessor RapidUI::GetDataAccessor(const tstring &name, bool recursive) const
{
	size_t i;
	size_t split=name.find('.');
	tstring rootName=name.substr(0, split);
	tstring memberName=name.substr(split+1);
	for (i=0; i<m_dataList.size(); ++i) {
		if (m_dataName.at(i)==name)
			return m_dataList.at(i);
		else {
			bool foundRoot=m_dataName.at(i)==rootName;
			if ((foundRoot || recursive) && m_dataList.at(i).is_aggregate()) {
				aggregate a=m_dataList.at(i).get_aggregate();
				pair<aggregate::iterator, bool> j=a.find_scope(t2string(foundRoot ? memberName : name));
				if (j.second==false)
					j=a.find_anywhere(t2string(foundRoot ? memberName : name));
				if (j.second)
					return *j.first;
			}
		}
	}
	return accessor();
}

wxWindow *RapidUI::FindWindow(wxWindow *searchThis, const wxString &name, bool recursive) const
{
	wxWindow *rc=0;
	if (GetWindowName(searchThis)==name)
		rc=searchThis;
	else if (recursive) {
		wxWindowList::compatibility_iterator n=searchThis->GetChildren().GetFirst();
		while (n && rc==0) {
			rc=FindWindow(n->GetData(), name, true);
			n=n->GetNext();
		}
	}
	return rc;
}

wxWindow *RapidUI::FindWindow(const tstring &name, bool recursive) const
{
	wxWindow *rc=0;
	WindowList::const_iterator i;
	for (i=m_windowList.begin(); i!=m_windowList.end() && rc==0; ++i) {
		rc=FindWindow(*i, name.c_str(), recursive);
	}
	return rc;
}

static bool begins_with(const tstring &prefix, const tstring &name)
{
	return prefix.length()>0 && name.substr(0, prefix.length())==prefix;
}

accessor RapidUI::GetAccessor(tstring name)
{
	wxLogRapidUI(("looking up variable %s", name.c_str()));
	accessor rc;
	bool searchWndOnly=false;
	bool searchDataOnly=false;
	if (begins_with(m_wndNamespace, name)) {
		searchWndOnly=true;
		name.erase(0, m_wndNamespace.length());
	} else if (begins_with(m_wndPrefix, name))
		searchWndOnly=true;
	else if (begins_with(m_dataNamespace, name)) {
		searchDataOnly=true;
		name.erase(0, m_dataNamespace.length());
	} else if (begins_with(m_dataPrefix, name))
		searchDataOnly=true;
	if (!searchDataOnly) {
		rc=GetWindowAccessor(name);
	}
	if (!rc.is_valid() && !searchWndOnly) {
		rc=GetDataAccessor(name);
	}
	if (!rc.is_valid())
		throw lwbase_error("GetAccessor couldn't find a variable '"+t2string(name)+"'");
	return rc;
}

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
wxString GetWindowFormatString(wxWindow *w)
{
	wxString rc;
	wxString wndName=w->GetName();
	size_t fpos=wndName.find(wxT(":F"));
	if (fpos!=wxString::npos) {
		if (wndName.at(fpos+2)=='=')
			++fpos;
		rc=wndName.substr(fpos+2);
	}
	return rc;
}

wxString GetWindowFormatProperties(wxWindow *w)
{
	wxString wndName=w->GetName();
	size_t fpos=wndName.find(wxT(":"));
	return fpos!=wxString::npos ? wndName.substr(fpos+1) : wxString();
}

wxString GetWindowName(wxWindow *w)
{
	wxString rc=w->GetName();
	int pos=rc.find(':');
	if (pos!=wxString::npos)
		rc.erase(pos);
	return rc;
}

};