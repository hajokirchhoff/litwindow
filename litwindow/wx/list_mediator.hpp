#ifndef wx_list_mediator_h__31080910
#define wx_list_mediator_h__31080910

#include "../ui/list_mediator.hpp"
#include "lwwx.h"
#include <wx/listctrl.h>
#include <boost/function.hpp>

namespace litwindow {
    namespace wx {

        class wxListCtrl_list_adapter:public ui::basic_list_adapter<wxListCtrl>
        {
            typedef wxListCtrl List;
            List    *m_ctrl;
        public:
            
        };

        class LWWX_API VirtualListCtrl:public wxListCtrl
        {
        public:
            typedef boost::function<wxString(long item, long column)> GetItemTextHandler;
            GetItemTextHandler on_get_item_text;

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
            virtual wxString OnGetItemText(long item, long column) const;
            //virtual int OnGetItemImage(long item) const;
            //virtual wxListItemAttr *OnGetItemColumnAttr(long item, long WXUNUSED) const;
            //virtual int OnGetItemColumnImage(long item, long column) const;
            DECLARE_DYNAMIC_CLASS(VirtualListCtrl);
        };

        inline ui::basic_list_adapter<VirtualListCtrl> make_list_adapter(VirtualListCtrl *l)
        {
            return ui::basic_list_adapter<VirtualListCtrl>();
        }
    }
}


#endif // list_mediator_h__31080910