#ifndef wx_list_mediator_h__31080910
#define wx_list_mediator_h__31080910

#include "../ui/list_mediator.hpp"
#include "lwwx.h"
#include <wx/listctrl.h>
#include <boost/function.hpp>
#include <boost/ref.hpp>
#include <boost/bind.hpp>

class wxDataViewCtrl;
class wxListBox;

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


        template <typename ColumnDescriptor>
        class wxColumns_traits
        {
        public:
            void insert_column(wxListCtrl *c, size_t index, const ColumnDescriptor &d) const
            {
                c->InsertColumn(index, d.title(), wxLIST_FORMAT_LEFT, d.width());
            }
            void remove_column(wxListCtrl *c, size_t index) const
            {
                c->DeleteColumn(index);
            }
            void set_column(wxListCtrl *c, size_t index, const ColumnDescriptor &d) const
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
                :m_ctrl(0) { set_control(l); }
            void set_control(VirtualListCtrl *l)
            {
                if (l!=m_ctrl) {
                    if (m_ctrl)
                        m_ctrl->on_get_item_text.clear();
                    m_ctrl=l;
                    l->on_get_item_text=boost::bind(&wxListCtrl_list_adapter::on_get_item_text, this, _1, _2);
                }
            }
            //~wxListCtrl_list_adapter() { if (m_ctrl) m_ctrl->on_get_item_text.clear(); }
            size_t column_count() const { return wnd()->GetColumnCount(); }
            size_t item_count() const { return wnd()->GetItemCount(); }
            void set_item_count(size_t new_count) { wnd()->SetItemCount(new_count); }
            template <typename ColumnsAdapter>
            void setup_columns(const ColumnsAdapter &d) const
            {
                ui::setup_columns(wxColumns_traits<typename ColumnsAdapter::column_descriptor_type>(), wnd(), d);
            }
            template <typename DatasetAdapter>
            void refresh_list(const DatasetAdapter &a)
            {
                set_item_count(a.size());
            }
            void begin_update() { wnd()->Freeze(); }
            void end_update() { wnd()->Thaw(); }

        private:
            wxString on_get_item_text(long /*item*/, long /*column*/)
            {
                //record_adapter i=m_mediator->get_record(item);
                //columns c=m_mediator->get_columns_adapter();
                //return c.get_text(i, c);
                return L"??";
            }
            //mediator *m_mediator;
            //wxColumns_traits m_columns_traits;
            value_type *m_ctrl;
            std::vector<size_t> m_visible_columns_index;
        };


        inline wxListCtrl_list_adapter make_list_adapter(VirtualListCtrl *l)
        {
            return wxListCtrl_list_adapter(l);
        }

        class wxDataViewCtrl_list_adapter
        {
            wxDataViewCtrl *m_ctrl;
        public:
            wxDataViewCtrl_list_adapter()
                :m_ctrl(0){}
            void set_control(wxDataViewCtrl *ctrl)
            {
                m_ctrl=ctrl;
            }
        };

        class wxListBox_list_adapter:public ui::basic_ui_control_adapter
        {
            wxControlWithItems *m_ctrl;
        public:
            wxListBox_list_adapter()
                :m_ctrl(0){}
            void set_control(wxControlWithItems *l)
            {
                m_ctrl=l;
            }
            wxControlWithItems *wnd() const { return m_ctrl; }
            template <typename DatasetAdapter>
            void refresh_list(const DatasetAdapter &a)
            {
                const DatasetAdapter::columns_adapter_type &columns_adapter(a.columns_adapter());
                wnd()->Clear();
                tstring text;
                size_t row=0;
                for (size_t i=0; i<a.size(); ++i) {
                    const DatasetAdapter::value_type &current(a.value_at(i));
                    if (columns_adapter.render_element_at(row, 0, *wnd(), current)==false) {
                        columns_adapter.render_element_at(0, text, current);
                        wnd()->Append(text);
                    }
                    ++row;
                }
            }

            template <typename ColumnsAdapter>
            void setup_columns(const ColumnsAdapter &c) const
            {
                return; // a list box does not have columns
            }
        };
    }
}


#endif // list_mediator_h__31080910