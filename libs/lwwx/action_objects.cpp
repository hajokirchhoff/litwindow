/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: action_objects.cpp,v 1.1 2006/09/11 14:10:40 Hajo Kirchhoff Exp $
 */
#include "stdwx.h"

using namespace std;

#include "litwindow/dataadapter.h"
#include "litwindow/wx/rapidUI.h"
#include "litwindow/logging.h"
#include "litwindow/wx/action_objects.h"
#include "litwindow/check.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef DOXYGEN_INVOKED

using namespace litwindow;

template <>
tstring converter<lwActionBase>::to_string(const lwActionBase &a)
{
	tostringstream s;
	s << a.GetActive() << a.GetEnabled();
	return s.str();
}

template <>
size_t converter<lwActionBase>::from_string(const tstring &newValue, lwActionBase &v)
{
	Precondition(newValue.length()==2, "lwActionBase::from_string invalid input value");
	v.SetEnabled(newValue[1]==_T('1'));
	v.SetActive(newValue==_T("11"));
	return 2;
}

//IMPLEMENT_ADAPTER_TYPE(litwindow::lwActionBase)
LWL_BEGIN_AGGREGATE_NO_COPY(litwindow::lwActionBase)
PROP_GetSet(bool, Enabled)
PROP_GetSet(bool, Active)
LWL_END_AGGREGATE()

#endif

namespace litwindow {
	void lwActionBase::SetActive(bool is_active)
	{
		if (m_active!=is_active) {
			m_active=is_active;
			NotifyChanged(*this, "Active", true);
		}
	}
	void lwActionBase::SetEnabled(bool is_enabled)
	{
		if (m_enabled!=is_enabled) {
			m_enabled=is_enabled;
			NotifyChanged(*this, "Enabled", true);
		}
	}
};
