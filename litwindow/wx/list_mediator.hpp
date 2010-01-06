#ifndef wx_list_mediator_h__31080910
#define wx_list_mediator_h__31080910

#include "../wx/basic_list_mediator.hpp"

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
		};
		template <typename UIControlPolicies>
		class basic_wxcontrol_with_columns_policies:public basic_wxcontrol_policies
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
		class uicontrol_policies:public basic_wxcontrol_policies
		{
		public:
			typedef UIControl uicontrol_type;
			//void insert_column(uicontrol_type *c, size_t idx, const ui::basic_column_label &d) {}
			//void set_column(uicontrol_type *c, size_t idx, const ui::basic_column_label &d) {}
			//void remove_column(uicontrol_type *c, size_t idx) {}
		};

		template <>
		class uicontrol_policies<wxListCtrl>:public basic_wxcontrol_with_columns_policies<uicontrol_policies<wxListCtrl> >
		{
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
		};

	}
	using wx::uicontrol_policies;
	//using namespace wx;
}

#endif // list_mediator_h__31080910
