#ifndef wx_list_mediator_h__31080910
#define wx_list_mediator_h__31080910

#include "../ui/list_mediator.hpp"
#include "lwwx.h"
#include <wx/listctrl.h>

namespace litwindow {
    namespace wx {

        class wxListCtrl_list_adapter:public ui::basic_list_adapter
        {
            typedef wxListCtrl List;
            List    *m_ctrl;
        public:
            
        };

        class LWWX_API VirtualListCtrl:public wxListCtrl
        {
        public:
            VirtualListCtrl():wxListCtrl() {}
            VirtualListCtrl(
                wxWindow *parent, 
                wxWindowID id, 
                const wxPoint &pos=wxDefaultPosition, 
                const wxSize &size=wxDefaultSize, 
                long style=wxLC_ICON,
                const wxValidator &validator=wxDefaultValidator,
                const wxString &name=wxListCtrlNameStr
                ):wxListCtrl(parent, id, pos, size, (style& ~ (wxLC_ICON|wxLC_LIST))|wxLC_VIRTUAL|wxLC_REPORT, validator, name) {}
            DECLARE_DYNAMIC_CLASS(VirtualListCtrl);
        };
    }
}


#endif // list_mediator_h__31080910