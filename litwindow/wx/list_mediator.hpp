#ifndef wx_list_mediator_h__31080910
#define wx_list_mediator_h__31080910

#include "../ui/list_mediator.hpp"
#include "lwwx.h"
#include <wx/listctrl.h>
#include <boost/function.hpp>
#include <boost/ref.hpp>

namespace litwindow {
    namespace wx {

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
        public:
        };

        class wxColumns_traits
        {
        public:
            void insert_column(wxListCtrl *c, size_t index, const ui::basic_column_descriptor &d) const
            {
                c->InsertColumn(index, d.title(), wxLIST_FORMAT_LEFT, d.width());
            }
            void remove_column(wxListCtrl *c, size_t index) const
            {
                c->DeleteColumn(index);
            }
            void set_column(wxListCtrl *c, size_t index, const ui::basic_column_descriptor &d) const
            {
                wxListItem it;
                it.SetText(d.title());
                it.SetWidth(d.width());
                c->SetColumn(index, it);
            }
            size_t count(wxListCtrl *c) const
            {
                return c->GetColumnCount();
            }
        };

        class wxListCtrl_list_adapter:public ui::basic_ui_control_adapter //:public ui::basic_ui_control_adapter<VirtualListCtrl, wxColumns_traits>
        {
        public:
            typedef VirtualListCtrl value_type;
            value_type *wnd() const { return m_ctrl; }
            wxListCtrl_list_adapter(VirtualListCtrl *l=0)
                :m_ctrl(l) {}
            size_t column_count() const { return wnd()->GetColumnCount(); }
            size_t item_count() const { return wnd()->GetItemCount(); }
            void setup_columns(const ui::basic_columns_adapter &d) const
            {
                ui::setup_columns(m_columns_traits, wnd(), d);
            }

        private:
            wxColumns_traits m_columns_traits;
            value_type *m_ctrl;
        };


        inline wxListCtrl_list_adapter make_list_adapter(VirtualListCtrl *l)
        {
            return wxListCtrl_list_adapter(l);
        }

    }
}


#endif // list_mediator_h__31080910