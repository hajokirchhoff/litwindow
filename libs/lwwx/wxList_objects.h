/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: wxList_objects.h,v 1.2 2006/04/04 09:21:33 Hajo Kirchhoff Exp $
*/
#ifndef _LW_WXLISTOBJECTS_
#define _LW_WXLISTOBJECTS_
#pragma once

#include <litwindow/lwbase.hpp>
#include <wx/listbox.h>
#include "./base_objects.h"

namespace litwindow {

/** @internal
@brief base class for extended adapters for wxListBox, wxComboBox.
This adapter adds RapidUI properties to wxListBox, wxComboBox.
*/
class lwListAdapterBase:public lwAdapterBase
{
public:
    const accessor &GetItems() const
    {
        return m_items;
    }
    void SetItems(const accessor &newValue);
    const accessor GetCurrent() const;
    void SetCurrent(const accessor &newValue);

    int GetValueAsInt() const;
    void SetValueAsInt(int newValue);

	virtual wxString GetValueAsString() const { return GetStringSelection(); }
	virtual void SetValueAsString(const wxString &str) { SetStringSelection(str); }

    const accessor GetValue() const;
    void SetValue(const accessor &newValue);

    const tstring &GetColumn() const
    {
        return m_column;
    }
    void SetColumn(const tstring &newValue)
    {
        m_column=newValue;
        FillList();
    }

    lwListAdapterBase():IndexClientData(false),m_is_enum(false),m_is_string(false),m_enum_info(0),m_enum_type(0) { }
    virtual ~lwListAdapterBase();

    virtual wxString GetStringSelection() const = 0;
    virtual void SetStringSelection(const wxString &str) = 0;
    virtual long GetClientDataSelection() const;
    virtual void SetClientDataSelection(long clientData);
    bool GetUseClientData() const { return IndexClientData; }
    void SetUseClientData(bool douse) { IndexClientData=true; }

    bool	IndexClientData;	///< if true, uses client data to index elements in the list
    bool    is_enum() const { return m_is_enum; }
	bool	is_string() const { return m_is_string; }
	bool	is_int() const { return !m_is_enum && !m_is_string; }
protected:
    virtual void FillList();
    virtual int GetCount() const = 0;
    virtual int GetSelection() const = 0;
    virtual void SetSelection(int newValue) = 0;
    virtual void *GetClientData(int index) const = 0;

    virtual void ClearList() = 0;
    virtual void AppendList(const tstring &element, void *clientData=0) = 0;
    virtual wxWindow *GetWnd() const = 0;
    virtual void CalcCurrent(int containerIndex) const;
    accessor    m_items;
    tstring      m_column;
    /// m_current is the accessor to the current item. It is delayed-created and thus mutable.
    mutable accessor    m_current;
    bool        m_is_enum;
	bool		m_is_string;
    void        set_is_enum() { m_is_enum=true; m_is_string=false; }
	void		set_is_string() { m_is_string=true; m_is_enum=false; }
	void		set_is_int() { m_is_string=m_is_enum=false; }
    /// temporary object to allow returning an accessor from GetValue
    mutable int m_int_value;
	mutable wxString m_string_value;
    const converter_enum_info *m_enum_info;
    litwindow::prop_t m_enum_type;
    DECLARE_DYNAMIC_CLASS_NO_COPY(lwListAdapterBase);
};

class wxListBoxAdapter:public lwListAdapterBase
{
    wxListBox   *m_wnd;
    DECLARE_ADAPTER(wxListBoxAdapter)
    DECLARE_EVENT_TABLE()

    //void FillList();
public:
    wxListBoxAdapter(wxListBox *l=0)
        :m_wnd(l)
    {
    }
    int GetCount() const 
    {
	    return m_wnd->GetCount(); 
    }
    void *GetClientData(int index) const
    {
	    return m_wnd->GetClientData(index);
    }
    wxWindow *GetWnd() const
    {
        return m_wnd;
    }
    void ClearList()
    {
        m_wnd->Clear();
    }
    void AppendList(const tstring &element, void *clientData)
    {
        m_wnd->Append(element.c_str(), clientData);
    }

    int GetSelection() const
    {
        return m_wnd->GetSelection();
    }
    void SetSelection(int newValue)
    {
        m_wnd->SetSelection(newValue);
    }

    wxString GetStringSelection() const
    {
        return m_wnd->GetStringSelection();
    }
    void SetStringSelection(const wxString &str)
    {
        m_wnd->SetStringSelection(str);
    }
    void OnListBox(wxCommandEvent &evt);

    DECLARE_DYNAMIC_CLASS_NO_COPY(wxListBoxAdapter);
};

class wxComboBoxAdapter:public lwListAdapterBase
{
    wxComboBox *m_wnd;
    DECLARE_ADAPTER(wxComboBoxAdapter)
    DECLARE_EVENT_TABLE()
    //void FillList();
public:
    wxComboBoxAdapter(wxComboBox *l=0)
        :m_wnd(l)
    {
    }
    int GetCount() const 
    {
	    return m_wnd->GetCount(); 
    }
    void *GetClientData(int index) const
    {
	    return m_wnd->GetClientData(index);
    }
    wxWindow *GetWnd() const
    {
        return m_wnd;
    }
    void ClearList()
    {
        m_wnd->Clear();
    }
    void AppendList(const tstring &element, void *clientData)
    {
        m_wnd->Append(element.c_str(), clientData);
    }

    int GetSelection() const
    {
        return m_wnd->GetSelection();
    }
    void SetSelection(int newValue)
    {
        m_wnd->SetSelection(newValue);
    }

    wxString GetStringSelection() const
    {
        return m_wnd->GetValue();
    }
    void SetStringSelection(const wxString &str)
    {
        m_wnd->SetValue(str);
    }
    void OnComboBox(wxCommandEvent &evt);
    void OnKillFocus(wxFocusEvent &evt);

    DECLARE_DYNAMIC_CLASS_NO_COPY(wxComboBoxAdapter);
};

class wxChoiceAdapter:public lwListAdapterBase
{
    wxChoice *m_wnd;
    DECLARE_ADAPTER(wxChoiceAdapter)
    DECLARE_EVENT_TABLE()
public:
    wxChoiceAdapter(wxChoice *l=0)
        :m_wnd(l)
    {
    }
    int GetCount() const 
    {
	    return m_wnd->GetCount(); 
    }
    void *GetClientData(int index) const
    {
	    return m_wnd->GetClientData(index);
    }
    wxWindow *GetWnd() const
    {
        return m_wnd;
    }
    void ClearList()
    {
        m_wnd->Clear();
    }
    void AppendList(const tstring &element, void *clientData)
    {
        m_wnd->Append(element.c_str(), clientData);
    }
    int GetSelection() const
    {
        return m_wnd->GetSelection();
    }
    void SetSelection(int newValue)
    {
        m_wnd->SetSelection(newValue);
    }

    wxString GetStringSelection() const
    {
        return m_wnd->GetStringSelection();
    }
    void SetStringSelection(const wxString &str)
    {
        m_wnd->SetStringSelection(str);
    }
    void OnChoice(wxCommandEvent &evt);

    DECLARE_DYNAMIC_CLASS_NO_COPY(wxChoiceAdapter);
};

inline wxListBoxAdapter *GetListBoxAdapter(wxListBox *w) { return GetAdapter<wxListBoxAdapter>(w, CLASSINFO(wxListBoxAdapter)); }
inline wxChoiceAdapter *GetChoiceAdapter(wxChoice *w) { return GetAdapter<wxChoiceAdapter>(w, CLASSINFO(wxChoiceAdapter)); }
inline wxComboBoxAdapter *GetComboBoxAdapter(wxComboBox *w) { return GetAdapter<wxComboBoxAdapter>(w, CLASSINFO(wxComboBoxAdapter)); }

};

#endif
