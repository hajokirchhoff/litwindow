#ifndef wx_list_mediator_h__31080910
#define wx_list_mediator_h__31080910

#include "../wx/basic_list_mediator.hpp"
#include "../ui/list_mediator.hpp"

#include <wx/listctrl.h>
namespace litwindow {
	namespace wx {

		class basic_wxcontrol_policies
		{
		public:
			typedef wxWindow uicontrol_type;
			void begin_update(uicontrol_type *c) { c->Freeze(); }
			void end_update(uicontrol_type *c) { c->Thaw(); }
			size_t column_count(uicontrol_type *c) const { return 1; }
			template <typename Mediator>
			void connect(Mediator *, uicontrol_type *) {}
			template <typename Mediator>
			void disconnect(Mediator *, uicontrol_type *) {}
			template <typename Mediator>
			void refresh_columns(Mediator &, uicontrol_type *) {}
		};
		template <typename UIControlPolicies>
		class basic_wxcontrol_with_rows_policies:public basic_wxcontrol_policies
		{
		public:
			UIControlPolicies* This() { return static_cast<UIControlPolicies*>(this); }
			template <typename Mediator>
			void refresh_rows(Mediator &m, typename Mediator::uicontrol_type *ctrl)
			{
				This()->remove_all_rows(ctrl);
				for (Mediator::const_iterator i=m.begin(); i!=m.end(); ++i) {
					This()->append_row(m, ctrl, i);
				}
			}
		};
		template <typename UIControlPolicies>
		class basic_wxcontrol_with_columns_policies:public basic_wxcontrol_with_rows_policies<UIControlPolicies>
		{
		public:
			UIControlPolicies* This() { return static_cast<UIControlPolicies*>(this); }
			template <typename Mediator>
			void refresh_columns(Mediator &m, typename Mediator::uicontrol_type *ctrl)
			{
				size_t idx=0;
				typename Mediator::columns_type &c(m.columns());
				while (idx<c.size()) {
					if (idx>=This()->column_count(ctrl))
						This()->insert_column(ctrl, idx, c.column_descriptor(idx));
					else
						This()->set_column(ctrl, idx, c.column_descriptor(idx));
					++idx;
				}
				while (column_count(ctrl)>c.size())
					This()->remove_column(ctrl, column_count(ctrl)-1);
			}
		};

		template <typename UIControl>
		class uicontrol_policies:public basic_wxcontrol_with_rows_policies<uicontrol_policies<UIControl> >
		{
		public:
			typedef UIControl uicontrol_type;
			void remove_all_rows(uicontrol_type *c)
			{
				c->Clear();
			}
			template <typename Mediator>
			void append_row(Mediator &m, uicontrol_type *ctrl, typename Mediator::const_iterator i)
			{
				ctrl->AppendString(m.as_string(i));
			}
		};

		template <>
		class uicontrol_policies<wxListCtrl>:public basic_wxcontrol_with_columns_policies<uicontrol_policies<wxListCtrl> >
		{
			typedef basic_wxcontrol_with_columns_policies<uicontrol_policies<wxListCtrl> > Inherited;
		public:
			typedef wxListCtrl uicontrol_type;
			size_t column_count(uicontrol_type *c) const { return c->GetColumnCount(); }
			void insert_column(uicontrol_type *c, size_t idx, const ui::basic_column_label &d)
			{
				c->InsertColumn(idx, d.title(), wxLIST_FORMAT_LEFT, d.width());
			}
			void set_column(uicontrol_type *c, size_t idx, const ui::basic_column_label &d) 
			{
				wxListItem it;
				it.SetText(d.title());
				it.SetWidth(d.width());
				c->SetColumn(idx, it);
			}
			void remove_column(uicontrol_type *c, size_t idx) 
			{
				c->DeleteColumn(idx);
			}
			void remove_all_rows(uicontrol_type *c)
			{
				c->DeleteAllItems();
			}
		};

		template <>
		class uicontrol_policies<VirtualListCtrl>:public uicontrol_policies<wxListCtrl>, public wxEvtHandler
		{
			typedef uicontrol_policies<wxListCtrl> Inherited;
		public:
			typedef VirtualListCtrl uicontrol_type;
			using Inherited::connect;
			using Inherited::disconnect;
			template <typename Mediator>
			void connect(Mediator *md, uicontrol_type* v)
			{
				v->on_get_item_text=boost::bind(&Mediator::get_item_text, md, _1, _2);
				on_destroyed=boost::bind(&Mediator::set_ui, md, (uicontrol_type*)0);
				v->Connect(wxEventType(wxEVT_DESTROY), wxObjectEventFunction(&uicontrol_policies::OnDestroy), 0, this);
			}
			template <typename Mediator>
			void disconnect(Mediator *, uicontrol_type *v)
			{
				v->Disconnect(wxEventType(wxEVT_DESTROY), wxObjectEventFunction(&uicontrol_policies::OnDestroy), 0, this);
				on_destroyed.clear();
			}
			template <typename Mediator>
			void refresh_rows(Mediator &m, typename Mediator::uicontrol_type *ctrl)
			{
				ctrl->SetItemCount(m.get_item_count());
			}
		protected:
			boost::function<void()> on_destroyed;
			void OnDestroy(wxEvent &evt)
			{
				evt.Skip();
				if (on_destroyed)
					on_destroyed();
			}
		};

	}
	using wx::uicontrol_policies;
	//using namespace wx;
}

#endif // list_mediator_h__31080910
