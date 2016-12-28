/* 
 * Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: wxio.h,v 1.4 2006/12/19 14:09:47 Hajo Kirchhoff Exp $
 */
#ifndef _LW_WXIO_
#define _LW_WXIO_
#pragma once
#include "litwindow/wx/lwwx.h"
#include "litwindow/dataadapter.h"
#include <wx/config.h>

namespace litwindow {

/** @addtogroup serializing_objects Serializing objects with data adapters.
    These methods serialize all properties of an object using wxConfigBase.
    Use these functions if you want to write or read the value of objects to or from
    a wxConfig object. This is commonly used to store and load user settings.
    @par Example: Reading values
    This example reads the values of a global @p g_settings variable from wxConfig using the path /settings.
    @code
    (*wxConfig::Get()) << SetPath("/settings") << make_const_accessor(g_settings);
    @endcode
    @par Example: Writing values
    This example writes the values of the @p g_settings variable to the wxConfig object.
    @code
    (*wxConfig::Get()) >> SetPath("/settings") >> make_accessor(g_settings);
    @endcode
    */
///@{

///@name Read/Write functions
///@{
/** Read values for the accessor from a Config object.
    This function will read all values for all members of the object, including containers and aggregates.
    If a value is not present in the configuration storage, the corresponding member will
    be left unchanged. There are no default values.
    @note Initialise the object to default values before using this function.
    */
wxConfigBase LWWX_API &operator >> (wxConfigBase &cfg, const accessor &destination);
inline wxConfigBase &operator >> (wxConfigBase &cfg, accessor &destination)
{
	return operator>>(cfg, static_cast<const accessor&>(destination));
}

template <typename Value>
inline wxConfigBase &operator >> (wxConfigBase &cfg, Value &destination)
{
	return operator>>(cfg, make_accessor(destination));
}

/** Write values for the accessor to a Config object.
    This function writes the values for all members of the object, including containers and aggregates,
    to the wxConfig object.
    */
wxConfigBase LWWX_API &operator << (wxConfigBase &cfg, const const_accessor &source);

template <typename Value>
inline wxConfigBase &operator<< (wxConfigBase &cfg, const Value &source)
{
	return operator<<(cfg, make_const_accessor(source));
}
///@}

/** @internal 
    Helper object that sets the path of a wxConfig object. */
struct LWWX_API SetPath
{
    SetPath(const TCHAR *newPath)
        :m_newPath(newPath)
    {
    }
    SetPath(const tstring &newPath)
	    :m_newPath(newPath)
    {
    }
    wxString    m_newPath;
};

typedef SetPath setpath;

/// @name Path setters
///@{
/// Changes the path to newPath
inline wxConfigBase LWWX_API &operator >> (wxConfigBase &cfg, const SetPath &newPath)
{
    cfg.SetPath(newPath.m_newPath);
    return cfg;
}

inline wxConfigBase LWWX_API &operator << (wxConfigBase &cfg, const SetPath &newPath)
{
    cfg.SetPath(newPath.m_newPath);
    return cfg;
}
///@}

///@}

};

#endif
