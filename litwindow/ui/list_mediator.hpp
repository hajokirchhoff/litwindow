#ifndef list_mediator_h__31080910
#define list_mediator_h__31080910

#include "basic_list_mediator.hpp"

namespace litwindow {
	namespace ui {

		typedef litwindow::tstring ui_string;

		//////////////////////////////////////////////////////////////////////////
		//------------------------------------------------------------------------------------------------------------------------------------
		template <typename Container, typename Value=typename Container::value_type>
		class container_policies
		{
		public:
			typedef Container container_type;
			typedef Value value_type;
			typedef typename container_type::iterator handle_type;
			typedef basic_column_descriptor<value_type> column_descriptor;
			typedef basic_columns_adapter<column_descriptor> columns_type;

			void refresh_handles(container_type &c) {}
		};

		template <typename Container>
		class stl_container_policies
		{
		public:
			typedef Container container_type;
			typedef typename container_type::value_type value_type;
			typedef typename container_type::iterator handle_type;
			typedef std::vector<handle_type> sorted_handles_t;
			typedef basic_column_descriptor<value_type> column_descriptor;
			typedef basic_columns_adapter<column_descriptor> columns_type;

			ui_string get_item_text(container_type &c, const columns_type &columns, size_t row, size_t column) const
			{
				const handle_type &h(get_row(row));
				ui_string rcstring;
				bool rc=columns.render_element_at(column, rcstring, handle_to_value(h));
				return rcstring;
			}

			void refresh_handles(container_type &c)
			{
				m_handles.resize(c.size());
				sorted_handles_t::iterator n=m_handles.begin();
				for (container_type::iterator i=c.begin(); i!=c.end(); ++i) {
					*n++=i;
				}
			}
			size_t size(container_type &c) const { return m_handles.size(); }
		protected:
			sorted_handles_t m_handles;
			const handle_type &get_row(size_t row) const { return m_handles[row]; }
			const value_type &handle_to_value(const handle_type &h) const { return *h; }
		};

		template <typename Value>
		class container_policies<std::vector<Value>, Value >
		{
		public:
		};

		template <typename Value>
		class container_policies<std::list<Value>, Value>:public stl_container_policies<std::list<Value> >
		{
			typedef stl_container_policies<std::list<Value> > Inherited;
		public:
		};

		template <typename Container, typename UIControl, typename ContainerPolicies=container_policies<Container>, typename UIControlPolicies=uicontrol_policies<UIControl> >
		class list_mediator
		{
		public:
			typedef typename ContainerPolicies::container_type container_type;
			typedef typename ContainerPolicies::columns_type columns_type;
			typedef typename UIControlPolicies::uicontrol_type uicontrol_type;

			void set_ui(uicontrol_type *ctrl)
			{ 
				if (m_uicontrol!=ctrl) {
					if (m_uicontrol)
						m_uicontrol_policies.disconnect(this, m_uicontrol);
					m_uicontrol=ctrl;
					set_dirty();
					if (ctrl)
						m_uicontrol_policies.connect(this, m_uicontrol);
				} 
			}
			uicontrol_type *get_ui() const { return m_uicontrol; }

			void set_container(container_type &ctnr)
			{ 
				if (&ctnr!=m_container) {
					m_container=&ctnr; set_dirty();
				} 
			}
			void set_dirty() 
			{
				m_dirty=true;
			}
			container_type &get_container() { return *m_container; }

			void columns(const columns_type &c) { m_columns=c; set_dirty(); m_columns.dirty(true); }
			columns_type &columns() { return m_columns; }

			litwindow::wstring get_item_text(size_t row, size_t col) const
			{
				return m_container_policies.get_item_text(*m_container, m_columns, row, col);
			}
			size_t get_item_count() const
			{
				return m_container_policies.size(*m_container);
			}

			void refresh() { do_refresh(); }
			void refresh(bool force) { if (force) { set_dirty(); m_columns.dirty(true); } do_refresh(); }
			~list_mediator()
			{
				set_ui(0);
			}
			list_mediator():m_dirty(false),m_uicontrol(0),m_container(0){}
			list_mediator(container_type &c, uicontrol_type *u)
			{ set_container(c); set_ui(u); }
		protected:
			columns_type m_columns;
			uicontrol_type *m_uicontrol;
			container_type *m_container;
			ContainerPolicies m_container_policies;
			UIControlPolicies m_uicontrol_policies;
			bool m_dirty;
			void do_refresh()
			{
				if (m_container && m_uicontrol && dirty()) {
					m_uicontrol_policies.begin_update(m_uicontrol);
					m_container_policies.refresh_handles(*m_container);
					if (m_columns.dirty()) {
						m_uicontrol_policies.refresh_columns(*this, m_uicontrol);
						m_columns.dirty(false);
					}
					m_uicontrol_policies.refresh_rows(*this, m_uicontrol);
					m_uicontrol_policies.end_update(m_uicontrol);
					m_dirty=false;
				}
			}

			bool dirty() 
			{
				return m_dirty || m_columns.dirty();
			}
		};
		
    }

};


#endif // list_mediator_h__31080910
