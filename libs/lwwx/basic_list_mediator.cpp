#include "stdwx.h"
#include "litwindow/wx/basic_list_mediator.hpp"

namespace litwindow {
    namespace wx {

        wxString VirtualListCtrl::OnGetItemText( long item, long column ) const
        {
            return on_get_item_text ? on_get_item_text(item, column) : wxString();
        }
		int VirtualListCtrl::OnGetItemColumnImage(long item, long column) const
		{
			return on_get_item_image ? on_get_item_image(item, column) : -1;
		}

    }
}
IMPLEMENT_DYNAMIC_CLASS(litwindow::wx::VirtualListCtrl, wxListCtrl);

wxDEFINE_EVENT(lwEVT_GET_LAYOUT_PERSPECTIVE, wxCommandEvent);
wxDEFINE_EVENT(lwEVT_SET_LAYOUT_PERSPECTIVE, wxCommandEvent);
