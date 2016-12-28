#ifndef litwindow_wx_wxGrid_mediator_h__
#define litwindow_wx_wxGrid_mediator_h__

#include "./list_mediator.hpp"

namespace litwindow
{
	namespace wx
	{
		#pragma once
		
		
		#pragma region wxGrid policies
		
				template <typename Mediator>
				class grid_table_base:public wxGridTableBase
				{
					Mediator *m_owner;
				public:
					grid_table_base(Mediator *owner):m_owner(owner){}
					~grid_table_base() {}
					virtual int GetNumberRows()
					{
						return (int)m_owner->size();
					}
		
					virtual int GetNumberCols()
					{
						return (int)m_owner->columns().size();
					}
		
					virtual wxString GetValue( int row, int col )
					{
						return m_owner->get_item_text(row, col);
					}
		
					virtual void SetValue( int row, int col, const wxString& value )
					{
					}

					virtual wxString GetColLabelValue( int col )
					{
						return m_owner->columns().at(col).title();
					}

					virtual bool InsertRows( size_t pos = 0, size_t numRows = 1 )
					{
						if ( GetView() )
						{
							wxGridTableMessage msg( this,
								wxGRIDTABLE_NOTIFY_ROWS_INSERTED,
								(int)pos,
								(int)numRows );

							GetView()->ProcessTableMessage( msg );
						}

						return true;
					}

					virtual bool AppendRows( size_t numRows = 1 )
					{
						if ( GetView() )
						{
							wxGridTableMessage msg( this,
								wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
								(int)numRows );

							GetView()->ProcessTableMessage( msg );
						}

						return true;
					}

					virtual bool DeleteRows( size_t pos = 0, size_t numRows = 1 )
					{
						if ( GetView() )
						{
							wxGridTableMessage msg( this,
								wxGRIDTABLE_NOTIFY_ROWS_DELETED,
								(int)pos,
								(int)numRows );

							GetView()->ProcessTableMessage( msg );
						}

						return true;
					}


				};
		
				template <>
				class uicontrol_policies<wxGrid>:public basic_wxcontrol_with_columns_policies<uicontrol_policies<wxGrid> >
				{
					typedef basic_wxcontrol_with_columns_policies<uicontrol_policies<wxGrid> > Inherited;
				public:
					typedef wxGrid uicontrol_type;
					template <typename Mediator>
					void connect(Mediator *md, uicontrol_type *v)
					{
						Inherited::connect(md, v);
						v->Bind(wxEVT_GRID_SELECT_CELL, &uicontrol_policies::OnGridSelectCell, this);
						v->SetTable(new grid_table_base<Mediator>(md), true);
					}
					template <typename Mediator>
					void refresh_columns(Mediator &m, typename Mediator::uicontrol_type *ctrl)
					{
						ctrl->SetTable(new grid_table_base<Mediator>(&m), true);
					}
					size_t column_count(uicontrol_type *c) const { return c->GetNumberCols(); }
/*
					void insert_column(uicontrol_type *c, size_t idx, const ui::basic_column_label &d)
					{
						long myidx=static_cast<long>(idx);
						c->InsertCols(myidx);
						c->SetColLabelValue(myidx, d.title());
						c->SetColumnWidth(myidx, d.width());
					}
*/
					template <typename Mediator>
					void append_row(Mediator &m, uicontrol_type *c, typename Mediator::const_iterator i)
					{
						c->AppendRows();
						c->SetCellValue(0, c->GetNumberRows()-1, m.as_string(i));
					}
/*
					void set_column(uicontrol_type *c, size_t idx, const ui::basic_column_label &d) 
					{
						long myidx=static_cast<long>(idx);
						c->SetColLabelValue(myidx, d.title());
						c->SetColumnWidth(myidx, d.width());
		// 				wxGridCellAttr *newa=new wxGridCellAttr();
		// 				newa->SetAlignment(d.)
		// 				c->SetColAttr(myidx, new wxGridCellAttr()
					}
*/

					void get_column(uicontrol_type *c, size_t idx, ui::basic_column_label &d)
					{
						if (d.visible())
							d.width(c->GetColSize(static_cast<long>(idx)));
					}

/*
					void remove_column(uicontrol_type *c, size_t idx) 
					{
						c->DeleteCols(static_cast<long>(idx));
					}
*/
					void remove_all_rows(uicontrol_type *c)
					{
						if (c->GetNumberRows()>0)
							c->DeleteRows(0, c->GetNumberRows());
					}
					size_t get_selection_index(uicontrol_type *c) const
					{
						wxGridCellCoordsArray a=c->GetSelectedCells();
						if (!a.empty())
							return a[0].GetRow();
						wxArrayInt rows=c->GetSelectedRows();
						if (!rows.empty())
							return rows[0];
						return size_t(-1);
					}
					template <typename Visitor>
					void for_each_selected(uicontrol_type *c, Visitor v) const
					{
						wxGridCellCoordsArray a=c->GetSelectedCells();
						for (int i=0; i<a.GetCount(); ++i) {
							v(a[i].GetRow());
						}
						wxArrayInt rows=c->GetSelectedRows();
						for (wxArrayInt::const_iterator i=rows.begin(); i!=rows.end(); ++i) {
							v(*i);
						}
					}
					void set_selection_index(uicontrol_type *c, size_t idx)
					{
						c->SelectRow(static_cast<long>(idx));
					}
					wxArrayInt columns_order(uicontrol_type *c)
					{
						wxArrayInt rc;
						for (int i=1; i<=c->GetNumberCols(); ++i)
							rc.push_back(c->GetColPos(i));
						return rc;
					}
					void columns_order(uicontrol_type *c, const wxArrayInt &order)
					{
						c->SetColumnsOrder(order);
					}
				protected:
					void OnGridSelectCell(wxGridEvent &evt)
					{
						evt.Skip();
					}
				};
		#pragma endregion
		
	}
}
#endif // litwindow_wx_wxGrid_mediator_h__
