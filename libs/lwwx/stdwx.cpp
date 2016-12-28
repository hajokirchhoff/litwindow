// stdafx.cpp : source file that includes just the standard includes
// src.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdwx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

/**@file
@addtogroup lwwxwidgets_todo Todo list for lwwxwidgets
    @todo
    -   True two way rules.
        Currently a two way rule is simulated by two one way rules. A <-> B is A <- B plus B <- A. This has the drawback that
        if B changes, A gets calculated but then, because A has been changed, B gets calculated again which isn't neccessary if
        its a two way rule.
    -   Remove or adapt the AddChangeMonitor mechanism.
        Currently the default rule for a wxTextCtrl requires the "wxTextCtrl.Value" entry in the defaultPropertyEventType map
        even though the Value property is handled by wxTextCtrlAdapter. Default rules should rely on the wxTextCtrlAdapter, not
        add another change monitor to the window.
    */