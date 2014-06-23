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

void litwindow::wx::VirtualListCtrl::Create( wxWindow *parent, wxWindowID id, const wxPoint &pos/*=wxDefaultPosition*/, const wxSize &size/*=wxDefaultSize*/, long style/*=wxLC_ICON*/, const wxValidator &validator/*=wxDefaultValidator*/, const wxString &name/*=wxListCtrlNameStr*/ )
{
	wxListCtrl::Create(parent, id, pos, size, (style& ~ (wxLC_ICON|wxLC_LIST))|wxLC_VIRTUAL|wxLC_REPORT, validator, name);
}

wxDEFINE_EVENT(lwEVT_GET_LAYOUT_PERSPECTIVE, wxCommandEvent);
wxDEFINE_EVENT(lwEVT_SET_LAYOUT_PERSPECTIVE, wxCommandEvent);
