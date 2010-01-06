#ifndef wx_basic_list_mediator_h__31080910
#define wx_basic_list_mediator_h__31080910

#include "../ui/basic_list_mediator.hpp"
#include "lwwx.h"
#include <wx/listctrl.h>
#include <wx/dataview.h>
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
            long GetFirstSelected() const
            {
                return GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
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
            void insert_column(wxDataViewCtrl *c, size_t index, const ColumnDescriptor &d) const
            {
                wxDataViewColumn *new_column=new wxDataViewColumn(d.title(), new wxDataViewTextRenderer(), index, d.width(), wxALIGN_LEFT);
                c->InsertColumn(index, new_column);
            }
            void remove_column(wxListCtrl *c, size_t index) const
            {
                c->DeleteColumn(index);
            }
            void remove_column(wxDataViewCtrl *c, size_t index) const
            {
                c->DeleteColumn(c->GetColumn(index));
            }
            void set_column(wxListCtrl *c, size_t index, const ColumnDescriptor &d) const
            {
                wxListItem it;
                it.SetText(d.title());
                it.SetWidth(d.width());
                c->SetColumn(index, it);
            }
            void set_column(wxDataViewCtrl *c, size_t index, const ColumnDescriptor &d) const
            {
                wxDataViewColumn *col=c->GetColumn(index);
                col->SetTitle(d.title());
                col->SetWidth(d.width());
            }
            size_t count(wxListCtrl *c) const
            {
                return c->GetColumnCount();
            }
            size_t count(wxDataViewCtrl *c) const
            {
                return c->GetColumnCount();
            }
        };

        class wxListCtrl_list_adapter:public ui::basic_ui_control_adapter
            //:public ui::basic_ui_control_adapter<VirtualListCtrl, wxColumns_traits>
        {
        public:
            typedef VirtualListCtrl value_type;
            value_type *wnd() const { return m_ctrl; }
            wxListCtrl_list_adapter(VirtualListCtrl *l=0)
                :m_ctrl(0) { set_control(l); }
            void set_control(VirtualListCtrl *l)
            {
                m_ctrl=l;
            }
            template <typename Mediator>
            void connect_mediator(const Mediator &m)
            {
                m_get_text=boost::bind(&Mediator::get_item_text, boost::cref(m), _1, _2, _3);
                m_ctrl->on_get_item_text=boost::bind(&wxListCtrl_list_adapter::on_get_item_text, this, _1, _2);
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
            template <typename DatasetAdapter, typename ColumnsAdapter>
            void refresh_list(const DatasetAdapter &a, const ColumnsAdapter &ca)
            {
                set_item_count(a.size());
            }
            void begin_update() { wnd()->Freeze(); }
            void end_update() { wnd()->Thaw(); }
            /// Get the index of the currently selected item.
            /// If the list control allows multiselect, this will return
            /// the first selected index.
            size_t get_selected_index() const;

        private:
            wxString on_get_item_text(long item, long column)
            {
                tstring rc;
                m_get_text(item, column, rc);
                return rc;
            }
            //mediator *m_mediator;
            //wxColumns_traits m_columns_traits;
            boost::function<void(long, long, tstring &)> m_get_text;
            value_type *m_ctrl;
            std::vector<size_t> m_visible_columns_index;
        };


        inline wxListCtrl_list_adapter make_list_adapter(VirtualListCtrl *l)
        {
            return wxListCtrl_list_adapter(l);
        }

        class wxDataViewCtrl_list_adapter;

        template <typename Mediator>
        class wxDataViewModel_adapter:public wxDataViewVirtualListModel
        {
        public:
            typedef Mediator mediator_type;
            typedef typename mediator_type::dataset_adapter_type dataset_adapter_type;
        private:
            mediator_type &m_owner;
            size_t m_current_item_count;
        public:
            wxDataViewModel_adapter(mediator_type &m)
                :m_owner(m), m_current_item_count(0){}
            virtual bool IsContainer(const wxDataViewItem &i) const { return false; }
            virtual wxDataViewItem GetParent(const wxDataViewItem &i) const { return wxDataViewItem(); }
            virtual unsigned int GetChildren(const wxDataViewItem &i, wxDataViewItemArray &children) const { children.clear(); return 0; }
            virtual unsigned int GetColumnCount() const { return m_owner.columns_adapter().size(); }
            virtual wxString GetColumnType(unsigned int col) const { return wxString("wxString"); }
            virtual void GetValue(wxVariant &variant, unsigned int row, unsigned int col) const
            {
                tstring text;
                m_owner.get_item_text(row, col, text);
                variant=text;
            }
            virtual bool SetValue(const wxVariant &variant, unsigned int row, unsigned int col)
            {
                return false;
            }
            void refresh(wxDataViewCtrl_list_adapter *)
            {
                const dataset_adapter_type &dataset(m_owner.dataset_adapter());
                size_t row=0;
                while (row<dataset.size() && row<m_current_item_count)
                    RowChanged(row++);
                while (row<dataset.size()) {
                    RowAppended();
                    ++row;
                    ++m_current_item_count;
                }
                while (m_current_item_count>row) {
                    RowDeleted(--m_current_item_count);
                }
            }
        };

        class wxDataViewCtrl_list_adapter:public ui::basic_ui_control_adapter
        {
        public:
            typedef wxDataViewCtrl value_type;
        private:
            value_type *m_ctrl;
            boost::function<void()> m_refresh_forwarder;
        public:
            value_type *wnd() const { return m_ctrl; }
            wxDataViewCtrl_list_adapter()
                :m_ctrl(0){}
            void set_control(wxDataViewCtrl *ctrl)
            {
                m_ctrl=ctrl;
            }
            template <typename Mediator>
            void connect_mediator(Mediator &m)
            {
                wxDataViewModel_adapter<Mediator> *model=new wxDataViewModel_adapter<Mediator>(m);
                m_refresh_forwarder=boost::bind(&wxDataViewModel_adapter<Mediator>::refresh, model, this);
                wnd()->AssociateModel(model);
                model->DecRef();
            }
            template <typename ColumnsAdapter>
            void setup_columns(const ColumnsAdapter &d) const
            {
                ui::setup_columns(wxColumns_traits<typename ColumnsAdapter::column_descriptor_type>(), wnd(), d);
            }
            template <typename DatasetAdapter, typename ColumnsAdapter>
            void refresh_list(const DatasetAdapter &a, const ColumnsAdapter &ca)
            {
                if (m_refresh_forwarder)
                    m_refresh_forwarder();
            }
        };

        class wxControlWithItems_list_adapter:public ui::basic_ui_control_adapter
        {
            wxControlWithItems *m_ctrl;
        public:
            wxControlWithItems_list_adapter(wxControlWithItems *l=0)
                :m_ctrl(l){}
            void set_control(wxControlWithItems *l)
            {
                m_ctrl=l;
            }
            wxControlWithItems *wnd() const { return m_ctrl; }
            template <typename DatasetAdapter, typename ColumnsAdapter>
            void refresh_list(const DatasetAdapter &a, const ColumnsAdapter &ca)
            {
                wnd()->Clear();
                tstring text;
                size_t row=0;
                for (size_t i=0; i<a.size(); ++i) {
                    const DatasetAdapter::value_type &current(a.value_at(i));
                    if (ca.render_element_at(row, 0, *wnd(), current)==false) {
                        ca.render_element_at(0, text, current);
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

        typedef wxControlWithItems_list_adapter wxListBox_list_adapter;
        typedef wxControlWithItems_list_adapter wxChoiceBox_list_adapter;

        inline wxListBox_list_adapter make_list_adapter(wxListBox *l)
        {
            return wxListBox_list_adapter(l);
        }
        inline wxChoiceBox_list_adapter make_list_adapter(wxChoice *l)
        {
            return wxChoiceBox_list_adapter(l);
        }

        inline size_t wxListCtrl_list_adapter::get_selected_index() const
        {
            long rc=wnd()->GetFirstSelected();
            return rc==-1 ? npos : rc;
        }

    }
    using wx::make_list_adapter;
}


#endif // list_mediator_h__31080910