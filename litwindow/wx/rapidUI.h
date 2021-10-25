/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Mikado Database System. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Mikado Database System 
* distribution, file LICENCE.TXT
* $Id: rapidUI.h,v 1.9 2008/03/05 17:32:21 Merry\Hajo Kirchhoff Exp $
*/
#ifndef _RAPIDUI_
#define _RAPIDUI_

#pragma once
#pragma warning(push)
#pragma warning(disable: 4275)

#include "litwindow/lwbase.hpp"
#include "litwindow/tstring.hpp"
#include "litwindow/dataadapter.h"
#include "litwindow/constraints.h"
#include "litwindow/wx/lwwx.h"
#include <wx/tokenzr.h>
#include <wx/window.h>

wxDECLARE_EXPORTED_EVENT(LWWX_API, lwEVT_COMMIT_PHASE, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(LWWX_API, lwEVT_VALUES_CHANGED, wxCommandEvent);

namespace litwindow {

//#pragma warning(disable:4660)
//LWWX_STL_VECTOR_EXPORT(tstring);
#ifdef not
#pragma warning(disable:4231)
LWWX_STL_VECTOR_EXPORT(wxWindow*);
#endif // not

class LWWX_API RapidUI;

typedef void (*rules_function_t)(RapidUI &);
//LWWX_STL_VECTOR_EXPORT(rules_function_t);

class LWWX_API RapidUI:public wxEvtHandler, public symbol_table_interface
{
public:

	RapidUI(void);
	~RapidUI(void);

	void AddWindow(wxWindow *w);
	void RemoveWindow(wxWindow *w);

	void AddData(const accessor &data, tstring name=tstring());

	void AddRules(rules_function_t r);

	void AssignTwoWay(const tstring &windowName, const tstring &dataName);
	void AssignTwoWay(wxWindow *w, const accessor &a);
	void AssignTwoWay(const accessor &value, const accessor &data);
	rule_base *Assign(rule_base *rule, const tstring &group);
	rule_base *Assign(const accessor &value, const const_accessor &data, const tstring &group);
	void AssignRules(const tstring &rules, const tstring &group, const tstring &from_source=_T("internal_source"));
	RapidUI& operator << (wxWindow *w)
	{
		AddWindow(w);
		return *this;
	}

	RapidUI& operator << (const accessor &data)
	{
		AddData(data);
		return *this;
	}

	RapidUI& operator << (rules_function_t r)
	{
		AddRules(r);
		return *this;
	}

	void Start();
	enum CommitPhase {
		commit_Immediate,
		commit_FocusLost,
		commit_Apply
	};
	void Apply(CommitPhase type=commit_Apply);
	void AddDefaultRules();
	//@{
	/** Transfer values from windows back to the data.
	*/
	void TransferDataFromWindow();
	void TransferWindowToData()
	{
		TransferDataFromWindow();
	}
	//@}
    void TransferDataToWindow();

	void Clear();
	void UnsolveAllWindowsToDataRules();
    void UnsolveAllDataToWindowsRules();

	/** Tells the solver that a value has changed.
	@return true if the value could be changed without violating constraints. false if a conflict occurred.
	*/
	bool ValueChanged(const const_accessor &value, bool recursive, bool solve_immediately=true);

	/** Get an accessor for a window attribute. 
	Passing a string in the form 'name.attribute' to the method returns an
	accessor to the attribute of a window of that name.
	If addEventMonitor is true, a change monitor will be added for the
	attribute so that ValueChanged will be called when this attribute
	changes.
	*/
	accessor GetWindowAccessor(tstring name, bool addChangeMonitor=true);

	/** Get an accessor for the variable 'name'. Resolve optional scope rules
	and look up the name in the corresponding window or data list.
	Scope rules are:
	-   "wnd::" prefix restricts search to the window storage
	-   "data::" prefix restricts search to the data storage
	-   If the name starts with the windows prefix, search the window storage.
	-   If the name starts with the data prefix, search in the data storage.
	-   If the name does not start with any prefix, search first in the window
	then in the data storage.
	*/
	accessor GetAccessor(tstring name);

	/** Set the window prefix for names. Names beginning with this prefix are
	as assumed to be located in the window storage. 
	*/
	void SetWindowPrefix(tstring wndPrefix)
	{
		m_wndPrefix=wndPrefix;
	}
	/** Set the data prefix for names. Names beginning with this prefix are
	as assumed to be located in the data storage. 
	*/
	void SetDataPrefix(tstring dataPrefix)
	{
		m_dataPrefix=dataPrefix;
	}

	/** Pause the RapidUI mechanism. */
	void Pause()
	{
		m_solver.disable();
	}
	/** Resume the RapidUI mechanism. */
	void Resume()
	{
		m_solver.enable();
	}

	/** notify all observers that this value has changed. */
	static void NotifyChanged(const const_accessor &value, bool recursive=false, bool solve_immediately=true);

	static RapidUI* GetRapidUI(wxWindow* ui_window);

	accessor GetDataAccessor(const tstring &name, bool recursive=true) const;
	wxWindow *FindWindow(const tstring &name, bool recursive=true) const;
	wxWindow *FindWindow(wxWindow *searchThis, const wxString &name, bool recursive) const;

	unsigned long GetCalcCycle() const { return m_solver.get_calc_cycle(); }
	accessor GetWindowAccessor(wxWindow *w, const tstring &attributeName);
protected:
	typedef set<RapidUI*> RapidUISet_t;

	static RapidUISet_t g_rapidUISet;

	virtual accessor lookup_variable(const string &name)
	{
		return GetAccessor(s2tstring(name));
	}
	tstring m_wndPrefix;
	tstring m_dataPrefix;
	tstring m_wndNamespace;
	tstring m_dataNamespace;

	void AddDefaultRulesForWindow(wxWindow *w);
	accessor GetWindowDefaultValueAccessor(wxWindow *w);
	accessor GetWindowAccessor(wxWindow *w);

	typedef vector<wxWindow*> WindowList;
	WindowList m_windowList;
	WindowList m_widgets_in_a_rule;

	typedef vector<accessor> DataList;
	DataList m_dataList;
	typedef vector<tstring> DataName;
	DataName m_dataName;

	typedef vector<rules_function_t> RulesList;
	RulesList m_rulesList;


	/// Create a unique name for accessor 'a'.
	tstring MakeUniqueName(const accessor &a) const;

	/// true if the default rules have been added.
	bool m_defaultRulesAdded;

	constraint_solver m_solver;

	void add_rules(const tstring &rules, const tstring &default_group, const tstring &source_url=_T("internal"));

private:
	RapidUI(const RapidUI &);
	void operator=(const RapidUI&);
};

template <typename Value>
RapidUI &operator << (RapidUI &r, Value &in)
{
	r.AddData(make_accessor(in));
	return r;
}

/** Sends a 'changed' notification for an aggregate to all observing RapidUI objects. */
template <class Value>
void NotifyChanged(const Value &v, const char *member, bool recursive=false, bool solve_immediately=true)
{
	RapidUI::NotifyChanged(make_const_aggregate(v)[member], recursive, solve_immediately);
}
#ifdef _UNICODE
/** Sends a 'changed' notification for an aggregate to all observing RapidUI objects. */
template <class Value>
void NotifyChanged(const Value &v, const TCHAR *member, bool recursive=false, bool solve_immediately=true)
{
	RapidUI::NotifyChanged(make_const_aggregate(v)[t2string(member)], recursive, solve_immediately);
}
#endif

/** Sends a 'changed' notification for a non-aggregate to all observing RapidUI objects. */
template <class Value>
void NotifyChanged(const Value &v, bool recursive=false, bool solve_immediately=true)
{
	RapidUI::NotifyChanged(make_const_accessor(v), recursive, solve_immediately);
}

inline void NotifyChanged(const const_accessor &value, bool recursive=false, bool solve_immediately=true)
{
	RapidUI::NotifyChanged(value, recursive, solve_immediately);
}
inline void NotifyChanged(const accessor &value, bool recursive=false, bool solve_immediately=true)
{
	RapidUI::NotifyChanged(value, recursive, solve_immediately);
}

#define BEGIN_RULES(name) \
	void name(litwindow::RapidUI &r) \
{

#define END_RULES() \
}

#define TWOWAY(a, b) \
	r.AssignTwoWay(r.GetAccessor(a), r.GetAccessor(b));

#define RULE(a, b) \
	r.Assign(make_rule(r.GetAccessor(a), b), wxT(""));

class GroupNames
{
	wxString m_names;
	wxString m_append;
	wxStringTokenizer t;
public:
	GroupNames(const wxString &names)
		:m_names(names)
	{
		if (!m_names.IsEmpty() && m_names[0u]=='[') {
			size_t endBracket=m_names.find(wxT("]."));
			if (endBracket!=wxString::npos) {
				m_append=wxString(wxT("."));
				m_append+=m_names.substr(endBracket+2);
				m_names=m_names.substr(1, endBracket-1);
			}
		}
		t.SetString(m_names, wxT(","), wxTOKEN_STRTOK);
	}
	wxString Next()
	{
		wxString rc=t.GetNextToken().Trim().Trim(false);
		if (!rc.IsEmpty())
			rc+=m_append;
		return rc;
	}
};

template <class E>
void add_group_rule(RapidUI &r, const wxString &target, const E &e)
{
	GroupNames m(target);
	wxString next;
	while ( !(next=m.Next()).IsEmpty() ) {
		r.Assign(make_rule(r.GetAccessor((const TCHAR*)next.c_str()), e), wxT(""));
	}
}

#define GROUP_RULE(a, b) \
	litwindow::add_group_rule(r, a, b);

#define ASSIGN(a, b) \
	r.Assign(r.GetAccessor(a), r.GetAccessor(b));

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
wxString LWWX_API GetWindowName(wxWindow *w);
wxString LWWX_API GetWindowFormatString(wxWindow *w);
wxString LWWX_API GetWindowFormatProperties(wxWindow *w);

};

#pragma warning(pop)
#endif
