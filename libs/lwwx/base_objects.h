/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: base_objects.h,v 1.2 2006/04/04 09:21:33 Hajo Kirchhoff Exp $
*/
#ifndef _LW_BASEOBJECTS_
#define _LW_BASEOBJECTS_
#pragma once

namespace litwindow {

class lwAdapterBase:public wxEvtHandler
{
public:
    lwAdapterBase() {}
    void OnWindowDestroy(wxWindowDestroyEvent &evt);
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS_NO_COPY(lwAdapterBase);
};

template <class Adapter, class Window>
Adapter *GetAdapter(Window *w  , wxClassInfo *type)
{
    wxEvtHandler *h=w->GetEventHandler();
    do {
        if (h->IsKindOf(type))
            return dynamic_cast<Adapter*>(h);
        h=h->GetNextHandler();
    } while (h);
    // not found. create a new one.
    Adapter *adapter=new Adapter(w);
    w->PushEventHandler(adapter);
    return adapter;
}

};

#endif
