#ifndef wx_list_mediator_h__31080910
#define wx_list_mediator_h__31080910

#include "../wx/basic_list_mediator.hpp"
#include "../ui/list_mediator.hpp"

#include <wx/listctrl.h>
#include <wx/grid.h>
namespace litwindow {
	namespace wx {

		using litwindow::ui::list_mediator;


//------------------------------------------------------------------------------------------------------------------------------------
		class basic_wxcontrol_policies:public wxEvtHandler
		{
		public:
			typedef wxWindow uicontrol_type;
			void begin_update(uicontrol_type *c) { c->Freeze(); }
			void end_update(uicontrol_type *c) { c->Thaw(); }
			size_t column_count(uicontrol_type *c) const { return 1; }
			template <typename Mediator>
			void connect(Mediator *m, uicontrol_type *v)
			{
				v->Connect(wxEventType(lwEVT_GET_LAYOUT_PERSPECTIVE), wxObjectEventFunction(&basic_wxcontrol_policies::OnGetLayoutPerspective), 0, this);
				v->Connect(wxEventType(lwEVT_SET_LAYOUT_PERSPECTIVE), wxObjectEventFunction(&basic_wxcontrol_policies::OnSetLayoutPerspective), 0, this);
				get_layout_perspective=boost::bind(&Mediator::get_layout_perspective, m, _1);
				set_layout_perspective=boost::bind(&Mediator::set_layout_perspective, m, _1);
			}
			template <typename Mediator>
			void disconnect(Mediator *m, uicontrol_type *v)
			{
				v->Disconnect(wxEventType(lwEVT_GET_LAYOUT_PERSPECTIVE), wxObjectEventFunction(&basic_wxcontrol_policies::OnGetLayoutPerspective), 0, this);
				v->Disconnect(wxEventType(lwEVT_SET_LAYOUT_PERSPECTIVE), wxObjectEventFunction(&basic_wxcontrol_policies::OnSetLayoutPerspective), 0, this);
				get_layout_perspective.clear();
				set_layout_perspective.clear();
			}
			template <typename Mediator>
			void refresh_columns(Mediator &, uicontrol_type *) {}
			template <typename Mediator>
			void get_columns(Mediator &, uicontrol_type *) {}
		private:
			boost::function<void(wstring &)> get_layout_perspective;
			boost::function<void(const wstring &)> set_layout_perspective;
			void OnGetLayoutPerspective(wxCommandEvent &evt)
			{
				wstring layout;
				get_layout_perspective(layout);
				evt.SetString(layout);
			}
			void OnSetLayoutPerspective(wxCommandEvent &evt)
			{
				set_layout_perspective(evt.GetString().wx_str());
			}
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
			template <typename Visitor>
			void for_each_selected(Visitor v)
			{

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
				wxArrayInt cols_order(c.size());
/*
				if (This()->column_count(ctrl)) {
					wxArrayInt cdefault(This()->column_count(ctrl));
					for (int i=0; i<cdefault.size(); ++i)
						cdefault[i]=i;
					ctrl->SetColumnsOrder(cdefault);
				}
*/
				while (idx<c.size()) {
					if (idx>=This()->column_count(ctrl))
						This()->insert_column(ctrl, idx, c.column_descriptor(idx));
					else
						This()->set_column(ctrl, idx, c.column_descriptor(idx));
					cols_order[idx]=c.column_descriptor(idx).position();
					if (cols_order[idx]==-1)
						cols_order[idx]=(int)idx;
					else if (cols_order[idx]!=idx)
						cols_order[idx]=cols_order[idx];
					++idx;
				}
				while (column_count(ctrl)>c.size())
					This()->remove_column(ctrl, column_count(ctrl)-1);
				if (!c.empty())
					This()->columns_order(ctrl, cols_order);
			}
			template <typename Mediator>
			void get_columns(Mediator &m, typename Mediator::uicontrol_type *ctrl)
			{
				size_t idx=0;
				typename Mediator::columns_type &c(m.columns());
				wxArrayInt cols_order;
				if (This()->column_count(ctrl)>0)
					cols_order=This()->columns_order(ctrl);
				while (idx<c.size() && idx<This()->column_count(ctrl)) {
					This()->get_column(ctrl, idx, c.column_descriptor(idx));
					if (idx<cols_order.size())
						c.column_descriptor(idx).position(cols_order[idx]);
					++idx;
				}
			}
			template <typename UIControl>
			wxArrayInt columns_order(UIControl *c)
			{
				return c->GetColumnsOrder();
			}
			template <typename UIControl>
			void columns_order(UIControl *c, const wxArrayInt &order)
			{
				c->SetColumnsOrder(order);
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
			size_t get_selection_index(uicontrol_type *ctrl) const
			{
				return ctrl->GetSelection();
			}
			void set_selection_index(uicontrol_type *ctrl, size_t new_selection)
			{
				ctrl->SetSelection(new_selection);
			}
			//template <typename Visitor>
			//void for_each_selected(Visitor v) const
			//{

			//}
		};

		/************************************************************************/
		/* wxListCtrl Policies                                                  */
		/************************************************************************/
		template <>
		class uicontrol_policies<wxListCtrl>:public basic_wxcontrol_with_columns_policies<uicontrol_policies<wxListCtrl> >
		{
			typedef basic_wxcontrol_with_columns_policies<uicontrol_policies<wxListCtrl> > Inherited;
		public:
			typedef wxListCtrl uicontrol_type;
			template <typename Mediator>
			void connect(Mediator *md, uicontrol_type* v)
			{
				Inherited::connect(md, v);
				v->Connect(wxEventType(wxEVT_COMMAND_LIST_COL_CLICK), wxListEventHandler(uicontrol_policies::OnListColClick), 0, this);
				on_destroyed=boost::bind(&Mediator::clear_ui, md);
				on_l_col_clicked = boost::bind(&Mediator::sort_by, md, _1, Mediator::sort_automatic);
				v->Connect(wxEventType(wxEVT_DESTROY), wxObjectEventFunction(&uicontrol_policies::OnDestroy), 0, this);
			}
			template <typename Mediator>
			void disconnect(Mediator *md, uicontrol_type *v)
			{
				if (!v->IsBeingDeleted()) {
					v->Disconnect(wxEventType(wxEVT_COMMAND_LIST_COL_CLICK), wxListEventHandler(uicontrol_policies::OnListColClick), 0, this);
					v->Disconnect(wxEventType(wxEVT_DESTROY), wxObjectEventFunction(&uicontrol_policies::OnDestroy), 0, this);
				}
				on_destroyed.clear();
			}
			void begin_update(uicontrol_type *c) { }
			void end_update(uicontrol_type *c)
			{
				int topItem=c->GetTopItem();
				int perPage=c->GetCountPerPage();
				int totalItems=c->GetItemCount();
				int bottomItem=min(totalItems, topItem+perPage);
				c->RefreshItems(topItem, bottomItem-1);
			}
			size_t column_count(uicontrol_type *c) const { return c->GetColumnCount(); }
			void insert_column(uicontrol_type *c, size_t idx, const ui::basic_column_label &d)
			{
				wxListItem it;
				it.SetText(d.title());
				it.SetWidth(d.visible() ? d.width() : 0);
				c->InsertColumn(static_cast<long>(idx), it);
			}
			void set_column(uicontrol_type *c, size_t idx, const ui::basic_column_label &d) 
			{
				wxListItem it;
				it.SetText(d.title());
				it.SetWidth(d.visible() ? d.width() : 0);
				it.SetAlign(wxLIST_FORMAT_LEFT);
				it.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_WIDTH | wxLIST_MASK_FORMAT);
				c->SetColumn(static_cast<long>(idx), it);
			}
			void get_column(uicontrol_type *c, size_t idx, ui::basic_column_label &d)
			{
				wxListItem it;
				it.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_WIDTH);
				c->GetColumn(static_cast<long>(idx), it);
				if (d.visible())
					d.width(it.GetWidth());
			}
			void remove_column(uicontrol_type *c, size_t idx) 
			{
				c->DeleteColumn(static_cast<long>(idx));
			}
			void remove_all_rows(uicontrol_type *c)
			{
				c->DeleteAllItems();
			}
			size_t get_selection_index(uicontrol_type *ctrl) const
			{
				return ctrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			}
			template <typename Visitor>
			void for_each_selected(uicontrol_type *ctrl, Visitor v) const
			{
				long idx=-1;
				while ((idx=ctrl->GetNextItem(idx, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED))!=-1) {
					v(idx);
				}
			}
			void set_selection_index(uicontrol_type *ctrl, size_t idx)
			{
				size_t current=get_selection_index(ctrl);
				if (current!=idx) {
					for_each_selected(ctrl, boost::bind(&uicontrol_type::SetItemState, ctrl, _1, 0, wxLIST_STATE_SELECTED));
					if (idx<(size_t)ctrl->GetItemCount()) {
						ctrl->SetItemState(static_cast<long>(idx), wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
						ctrl->EnsureVisible(static_cast<long>(idx));
					}
				}
			}
		protected:
			boost::function<void()> on_destroyed;
			boost::function<void(int col)> on_l_col_clicked;
			void OnDestroy(wxEvent &evt)
			{
				evt.Skip();
				if (on_destroyed)
					on_destroyed();
			}
			void OnListColClick(wxListEvent &evt)
			{
				if (on_l_col_clicked)
					on_l_col_clicked(evt.GetColumn());
			}
		};

		template <>
		class uicontrol_policies<VirtualListCtrl>:public uicontrol_policies<wxListCtrl>
		{
			typedef uicontrol_policies<wxListCtrl> Inherited;
			bool m_right_click_in_progress;
		public:
			//boost::function<void(int)> toggle_show_column;
			typedef VirtualListCtrl uicontrol_type;
			using Inherited::connect;
			using Inherited::disconnect;
			//template <typename Mediator>
			template <typename Mediator>
			void OnRightClickMenu(Mediator *md, wxCommandEvent &evt)
			{
				if (m_right_click_in_progress) {
					int col=evt.GetId()-10000;
					md->toggle_show_column(col);
				} else
					evt.Skip();
			}
			template <typename Mediator>
			void OnColumnRightClick(Mediator *md, wxCommandEvent &evt)
			{
				wxListCtrl *v=dynamic_cast<wxListCtrl*>(evt.GetEventObject());
				if (v) {
					wxMenu menu;
					for (int i=0; i<v->GetColumnCount(); ++i) {
						wxListItem col;
						col.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_WIDTH);
						v->GetColumn(i, col);
						if (col.GetWidth()!=0 || md->columns().at(i).visible())
							md->columns().at(i).width(col.GetWidth());
						wxMenuItem *mi=menu.AppendCheckItem(10000+i, col.GetText());
						mi->Check(col.GetWidth()>0);
					}
					m_right_click_in_progress=true;
					v->PopupMenu(&menu);
					m_right_click_in_progress=false;
				} else
					evt.Skip();
			}
			template <typename Mediator>
			void connect(Mediator *md, uicontrol_type* v)
			{
				m_right_click_in_progress=false;
				Inherited::connect(md, v);
				v->on_get_item_text=boost::bind(&Mediator::get_item_text, md, _1, _2);
				v->on_get_item_image=boost::bind(&Mediator::get_item_image, md, _1, _2);
				v->Bind(wxEVT_COMMAND_LIST_COL_RIGHT_CLICK, boost::bind(&uicontrol_policies::OnColumnRightClick<typename Mediator>, this, md, _1));
				//toggle_show_column=boost::bind(&Mediator::toggle_show_column, md, _1);
				//v->Bind(wxEVT_COMMAND_MENU_SELECTED, boost::bind(&uicontrol_policies<VirtualListCtrl>::OnRightClickMenu, this, _1));
				v->Bind(wxEVT_COMMAND_MENU_SELECTED, boost::bind(&uicontrol_policies<VirtualListCtrl>::OnRightClickMenu<typename Mediator>, this, md, _1));
				v->Bind(wxEVT_ERASE_BACKGROUND, boost::bind(&uicontrol_policies<VirtualListCtrl>::OnEraseBackground<typename Mediator>, this, _1));
			}
			template <typename Mediator>
			void disconnect(Mediator *md, uicontrol_type *v)
			{
				Inherited::disconnect(md, v);
			}
			template <typename Mediator>
			void refresh_rows(Mediator &m, typename Mediator::uicontrol_type *ctrl)
			{
				if (ctrl->GetItemCount() != (long)m.get_item_count())
					ctrl->SetItemCount((long)m.get_item_count());
			}
			template <typename Mediator>
			void OnEraseBackground(wxEraseEvent & event) {
				// to prevent flickering, erase only content *outside* of the 
				// actual list items stuff
				typename Mediator::uicontrol_type *ctrl = dynamic_cast<Mediator::uicontrol_type*>(event.GetEventObject());
				if (ctrl && ctrl->GetItemCount() > 0) {
					wxDC * dc = event.GetDC();
					assert(dc);

					// get some info
					wxCoord width = 0, height = 0;
					ctrl->GetClientSize(&width, &height);

					wxCoord x, y, w, h;
					dc->SetClippingRegion(0, 0, width, height);
					dc->GetClippingBox(&x, &y, &w, &h);

					long top_item = ctrl->GetTopItem();
					long bottom_item = top_item + ctrl->GetCountPerPage();
					if (bottom_item >= ctrl->GetItemCount()) {
						bottom_item = ctrl->GetItemCount() - 1;
					}

					// trick: we want to exclude a couple pixels
					// on the left side thus use wxLIST_RECT_LABEL
					// for the top rect and wxLIST_RECT_BOUNDS for bottom
					// rect
					wxRect top_rect, bottom_rect;
					ctrl->GetItemRect(top_item, top_rect, wxLIST_RECT_LABEL);
					ctrl->GetItemRect(bottom_item, bottom_rect, wxLIST_RECT_BOUNDS);

					// set the new clipping region and do erasing
					wxRect items_rect(top_rect.GetLeftTop(), bottom_rect.GetBottomRight());
					wxRegion reg(wxRegion(x, y, w, h));
					reg.Subtract(items_rect);
					dc->DestroyClippingRegion();
					dc->SetDeviceClippingRegion(reg);

					// do erasing
					dc->SetBackground(wxBrush(ctrl->GetBackgroundColour(), wxSOLID));
					dc->Clear();

					// restore old clipping region
					dc->DestroyClippingRegion();
					dc->SetDeviceClippingRegion(wxRegion(x, y, w, h));
				}
				else {
					event.Skip();
				}
			}
		};


		//////////////////////////////////////////////////////////////////////////
		//------------------------------------------------------------------------------------------------------------------------------------
#pragma region wxDataViewCtrl policies
		//------------------------------------------------------------------------------------------------------------------------------------
		
		template <typename Mediator>
		class wxDataViewModel_policies:public wxDataViewVirtualListModel
		{
		public:
			typedef Mediator mediator_type;
		private:
			mediator_type &m_owner;
			size_t m_current_item_count;
		public:
			wxDataViewModel_policies(mediator_type &m)
				:m_owner(m), m_current_item_count(0){}
			virtual bool IsContainer(const wxDataViewItem &i) const { return false; }
			virtual wxDataViewItem GetParent(const wxDataViewItem &i) const { return wxDataViewItem(); }
			virtual unsigned int GetChildren(const wxDataViewItem &i, wxDataViewItemArray &children) const { children.clear(); return 0; }
			virtual unsigned int GetColumnCount() const { return m_owner.columns().size(); }
			virtual wxString GetColumnType(unsigned int col) const { return wxString("wxString"); }
			virtual void GetValue(wxVariant &variant, unsigned int row, unsigned int col) const
			{
				tstring text;
				text=m_owner.get_item_text(row, col);
				variant=text;
			}
			virtual void GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const
			{
				GetValue(variant, row, col);
			}
			virtual bool SetValue(const wxVariant &variant, unsigned int row, unsigned int col)
			{
				return false;
			}
			virtual bool SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col)
			{
				return SetValue(variant, row, col);
			}
			void refresh()
			{
				Reset(m_owner.size());
			}
		};		
		
		template <>
		class uicontrol_policies<wxDataViewCtrl>:public basic_wxcontrol_with_columns_policies<uicontrol_policies<wxDataViewCtrl> >, public boost::noncopyable
		{
			typedef basic_wxcontrol_with_columns_policies<uicontrol_policies<wxDataViewCtrl> > Inherited;
		public:
			typedef wxDataViewCtrl uicontrol_type;
			template <typename Mediator>
			void connect(Mediator *md, uicontrol_type* v)
			{
				Inherited::connect(md, v);
				wxDataViewModel_policies<Mediator> *model=new wxDataViewModel_policies<Mediator>(*md);
				v->AssociateModel(model);
				model->DecRef();
			}
			template <typename Mediator>
			void refresh_rows(Mediator &m, uicontrol_type *ctrl)
			{
				wxDataViewModel_policies<Mediator> *model(dynamic_cast<wxDataViewModel_policies<Mediator>*>(ctrl->GetModel()));
				model->refresh();
			}
			size_t column_count(uicontrol_type *c) const { return c->GetColumnCount(); }
			void insert_column(uicontrol_type *c, size_t idx, const ui::basic_column_label &d)
			{
				wxDataViewColumn *new_column=new wxDataViewColumn(d.title(), new wxDataViewTextRenderer(), static_cast<long>(idx), d.width(), wxALIGN_LEFT);
				c->InsertColumn(static_cast<long>(idx), new_column);
			}
			void set_column(uicontrol_type *c, size_t idx, const ui::basic_column_label &d) 
			{
				wxDataViewColumn *col=c->GetColumn(static_cast<long>(idx));
				col->SetTitle(d.title());
				col->SetWidth(d.visible() ? d.width() : -d.width());
			}
			void remove_column(uicontrol_type *c, size_t idx) 
			{
				c->DeleteColumn(c->GetColumn(static_cast<long>(idx)));
			}
		};
#pragma endregion wxDataViewCtrl policies
		//------------------------------------------------------------------------------------------------------------------------------------
	}
	using wx::uicontrol_policies;
	//using namespace wx;
}

#endif // list_mediator_h__31080910
