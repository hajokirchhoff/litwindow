/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: action_objects.h,v 1.2 2007/04/10 09:35:01 Hajo Kirchhoff Exp $
*/
#ifndef _LWL_ACTION_OBJECTS_
#define _LWL_ACTION_OBJECTS_
#pragma once

#include <wx/wxprec.h>

namespace litwindow {

	class lwActionBase
	{
	public:
		lwActionBase():m_active(false),m_enabled(true) {}
		virtual void SetActive(bool is_active);
		virtual bool GetActive() const { return m_active; }
		virtual void SetEnabled(bool is_enabled);
		virtual bool GetEnabled() const { return m_enabled; }
		/// Set a text to show to the user
		virtual void SetLabel(const tstring &label);
		virtual tstring GetLabel() const;
		/// Set an image to show
		virtual void SetImage(const wxImage &img);
		virtual wxImage GetImage() const;
		/// Set an action name. This is the unique name for the action and will be used to create an XRCID
		virtual void SetName(const tstring &name);
		virtual tstring GetName() const;

		int		GetId() const;
	protected:
		bool m_active;
		bool m_enabled;
	};
};

#endif
