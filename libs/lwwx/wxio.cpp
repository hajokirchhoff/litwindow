/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: wxio.cpp,v 1.7 2007/04/19 04:29:34 Hajo Kirchhoff Exp $
 */
#include "stdwx.h"
#include "litwindow/wx/wxio.h"
#include "litwindow/renderer.hpp"
#include <wx/datetime.h>
#include <sstream>
#include <iomanip>
using namespace std;

namespace litwindow {

	namespace {
		void render_wxdatetime(tstring &out, const const_accessor &in, const tstring &format)
		{
			wxDateTime value(dynamic_cast_accessor<wxDateTime>(in).get());
			if (value.IsValid())
				out=value.Format(format.c_str());
			else
				out.clear();
		}
		dataadapter::register_renderer<wxDateTime, tstring> register_datetime_r(&render_wxdatetime, _T("%c"), dataadapter::renderer<tstring>::get());

		void render_wxtimespan(tstring &out, const const_accessor &in, const tstring &format)
		{
			wxDateTime datetime(wxDateTime::Today()+dynamic_cast_accessor<wxTimeSpan>(in).get());
			render_wxdatetime(out, make_const_accessor(datetime), format);
		}
		dataadapter::register_renderer<wxTimeSpan, tstring> register_timespan_r(&render_wxtimespan, _T("%X"), dataadapter::renderer<tstring>::get());

	};

template <class Fnc, class _Ac>
Fnc for_each_nested(_Ac a, Fnc f, tstring path=wxT(""), const tstring &separator=wxT("."), bool flatten_container=false)
{
    if (a.is_aggregate()) {
        _Ac::aggregate_type ag=a.get_aggregate();
        _Ac::aggregate_type::iterator_type begin=ag.begin();
        _Ac::aggregate_type::iterator_type end=ag.end();
        while (begin!=end) {
            f=for_each_nested(*begin, f, path + (path.length()>0 ? separator : wxT("")) + s2tstring(begin->name()), separator, flatten_container);
            ++begin;
        }
    } else
        f(a, path);
    return f;
}

struct read_from_config
{
    read_from_config(wxConfigBase *cfg)
        :m_cfg(cfg)
    {
    }
    void operator()(const accessor &value, const tstring &path)
    {
	    value.assert_valid();
	    wxString key=path.c_str();
	    if (key.IsEmpty())
		    key=s2tstring(value.name()).c_str();
	    bool success=false;
	    if (value.is_container()) {
		    wxString currentPath=m_cfg->GetPath();
		    wxString container_path(currentPath+wxT("/")+key);
		    m_cfg->SetPath(container_path);
		    // reading containers allows two different schemes
		    // the first scheme - as used by wxio - stores the size of the container in .../size and counts from 0 on
		    // the second scheme - used with ini files that have been created manually - simply uses all existing groups in alphabetical order
		    long size;
		    bool use_counting=m_cfg->Read(wxT("size"), &size);
		    vector<wxString> group_names_if_not_counting;
		    if (use_counting==false) {
			    long cookie;
			    wxString group;
			    bool has_more=m_cfg->GetFirstGroup(group, cookie);
			    while (has_more) {
				    group_names_if_not_counting.push_back(group);
				    has_more=m_cfg->GetNextGroup(group, cookie);
			    }
			    size=group_names_if_not_counting.size();
		    }
		    if (size>0) {
			    container c=value.get_container();
			    accessor theValue;
			    try {
				    long count;
				    container::iterator insertionPoint=c.begin();
				    for (count=0;  count<size; ++count) 
				    {
					    wxString group(container_path);
					    group.append(_T("/"));
					    if (use_counting)
						    group.append(wxString::Format(_T("%010d"), count));
					    else
						    group.append(group_names_if_not_counting.at(count));
					    if (m_cfg->HasGroup(group)==false) {
						    wxLogTrace(wxT("wxio"), wxT("Warning: element #%d is missing from configuration storage. Ignoring."), count);
					    } else {
						    m_cfg->SetPath(group);
						    theValue=create_object(c.get_element_type());
						    (*m_cfg) >> theValue;
						    c.insert(insertionPoint, theValue);
						    theValue.destroy();
					    }
				    }
			    }
			    catch (...) {
				    theValue.destroy();
			    }
		    }
		    m_cfg->SetPath(currentPath);
		    success=true;
	    } else {
		    if (!m_cfg->HasEntry(key)) {
			    wxLogTrace(wxT("wxio"), wxT("%s is missing from configuration storage. Ignoring."), key.c_str());
			    success=true;
		    } else {
			    try {
				    if (value.is_int()) {
					    long l;
					    if (m_cfg->Read(key, &l)) {
						    value.from_int(l);
						    success=true;
					    }
				    } else {
					    wxString data;
					    if (m_cfg->Read(key, &data)) {
						    value.from_string((const TCHAR*)data.c_str());
						    success=true;
					    }
				    }
			    }
			    catch (lwbase_error &) {
				    /// ignore errors
			    }
		    }
		    if (!success)
			    wxLogError(wxT("Reading %s from configuration storage failed!"), key.c_str());
	    }
    }
    wxConfigBase *m_cfg;
};

wxConfigBase &operator >> (wxConfigBase &cfg, const accessor &a)
{
    for_each_nested(a, read_from_config(&cfg), wxT(""), wxT("/"), true);
    return cfg;
}

struct write_to_config
{
    write_to_config(wxConfigBase *cfg)
        :m_cfg(cfg)
    {
    }
    void operator()(const const_accessor &value, const tstring &path)
    {
        value.assert_valid();
        bool success=false;
        wxString key=path.c_str();
        if (key.IsEmpty())
            key=s2tstring(value.name()).c_str();
        if (value.is_container()) {
            long count=0;
            const_container c=value.get_container();
            const_container::const_iterator begin=c.begin();
            const_container::const_iterator end=c.end();
            wxString currentPath=m_cfg->GetPath();
            // remove all container entries in case the container has shrunk
            m_cfg->DeleteGroup(key);
            while (begin!=end) {
                tostringstream s;
                s << currentPath << wxT("/") << key << wxT("/") << setw(10) << setfill(wxT('0')) << count;
                m_cfg->SetPath(s.str().c_str());
                (*m_cfg) << *begin;
                ++begin;
                ++count;
            }
            m_cfg->SetPath(currentPath);
            success=m_cfg->Write(key+wxT("/size"), count);
        } else {
            try {
                if (value.is_int()) {
                    if (m_cfg->Write(key, value.to_int())) {
                        success=true;
                    }
                } else {
                    if (m_cfg->Write(key, value.to_string().c_str())) {
                        success=true;
                    }
                }
            }
            catch (lwbase_error &) {
                /// ignore errors
            }
        }
        if (!success)
            wxLogError(wxT("Writing %s to configuration storage failed!"), key.c_str());
    }
    wxConfigBase *m_cfg;
};

wxConfigBase &operator << (wxConfigBase &cfg, const const_accessor &a)
{
    for_each_nested(a, write_to_config(&cfg), wxT(""), wxT("/"), true);
    return cfg;
}


};