#include "stdwx.h"
#include "litwindow/wx/basic_list_mediator.hpp"

namespace litwindow {
    namespace wx {

        wxString VirtualListCtrl::OnGetItemText( long item, long column ) const
        {
            return on_get_item_text ? on_get_item_text(item, column) : wxString();
        }

    }
}
IMPLEMENT_DYNAMIC_CLASS(litwindow::wx::VirtualListCtrl, wxListCtrl);
