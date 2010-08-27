#ifndef list_mediator_h__31080910
#define list_mediator_h__31080910

#include "basic_list_mediator.hpp"
#include <deque>
#include <list>
#include <boost/shared_ptr.hpp>

namespace litwindow {
	namespace ui {

		typedef litwindow::tstring ui_string;

		//////////////////////////////////////////////////////////////////////////
		template <typename ColumnDescriptor, typename HandlePolicies>
		struct basic_columns_sorter {
			typedef typename ColumnDescriptor::value_type value_type;
			typedef typename HandlePolicies handle_policies_type;
			typedef typename handle_policies_type::handle_type handle_type;
			struct sort_column
			{
				int m_column_index;
				bool m_sort_ascending;
				const ColumnDescriptor *m_column_descriptor;
				sort_column()
					:m_column_index(-1){}
				sort_column(int idx, bool ascending, const ColumnDescriptor &d)
					:m_column_index(idx),m_sort_ascending(ascending),m_column_descriptor(&d){}
				bool is_valid() const { return m_column_index>=0; }
			};
			typedef std::deque<sort_column> sort_columns_t;
			sort_columns_t m_sort_columns;
			handle_policies_type m_handle_policies;
			basic_columns_sorter():m_sort_columns(3){}
			void clear()
			{
				std::fill(m_sort_columns.begin(), m_sort_columns.end(), sort_column());
			}
			void push_sort(int new_column, const ColumnDescriptor &d)
			{
				sort_columns_t::iterator i=find_if(m_sort_columns.begin(), m_sort_columns.end(), boost::bind(&sort_column::m_column_index, _1)==new_column);
				bool sortascending;
				if (i==m_sort_columns.end()) {
					sortascending=true;
					m_sort_columns.pop_back();
				} else {
					if (i==m_sort_columns.begin())
						sortascending=!i->m_sort_ascending;
					else
						sortascending=true;
					m_sort_columns.erase(i);
				}
				m_sort_columns.push_front(sort_column(new_column, sortascending, d));
			}
			bool compare(const value_type &left, const value_type &right) const
			{
				sort_columns_t::const_iterator i=m_sort_columns.begin();
				while (i!=m_sort_columns.end() && i->is_valid()) {
					const sort_columns_t::value_type &current(*i);
					const value_type *l, *r;
					if (i->m_sort_ascending) { l=&left; r=&right; } else { l=&right; r=&left; }
					if (i->m_column_descriptor->compare(*l, *r, i->m_column_index))
						return true;	// left < right if ascending == true and left > right if ascending == false
					if (i->m_column_descriptor->compare(*r, *l, i->m_column_index))
						return false;	// left > right if ascending == true and left < right if ascending == false
					// if control flow arrives here, then   !(left<right) && !(left>right)
					// which means left == right. So check next column if there is one.
					++i;
				}
				return false;	// all columns tested equal, so left is not less than right
			}
			bool compare(const value_type *left, const value_type *right) const { return compare(*left, *right); }
			bool operator()(handle_type left, handle_type right) const { return compare(m_handle_policies.handle_to_value(left), m_handle_policies.handle_to_value(right)); }
		};
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
		class handle_policies
		{
		public:
			typedef typename Container container_type;
			typedef typename container_type::value_type value_type;
			typedef typename container_type::iterator handle_type;
			const value_type &handle_to_value(const handle_type &h) const { return *h; }
		};

		template <typename Value>
		class handle_policies<std::vector<boost::shared_ptr<Value> > >
		{
		public:
			typedef std::vector<boost::shared_ptr<Value> > container_type;
			typedef typename Value value_type;
			typedef typename container_type::iterator handle_type;
			const value_type &handle_to_value(const handle_type &h) const { return **h; }
		};

		template <typename Container, typename HandlePolicies=handle_policies<Container> >
		class stl_container_policies
		{
		public:
			static const bool insert_stable=false;
			static const bool erase_stable=true;
			typedef stl_container_policies container_policies_type;
			typedef Container container_type;
			typedef HandlePolicies handle_policies_type;
			typedef typename handle_policies_type::value_type value_type;
			typedef typename handle_policies_type::handle_type handle_type;
			typedef std::vector<handle_type> sorted_handles_t;
			typedef basic_column_descriptor<value_type> column_descriptor;
			typedef basic_columns_adapter<column_descriptor> columns_type;

			typedef typename sorted_handles_t::const_iterator const_iterator;
			typedef typename sorted_handles_t::iterator iterator;

			int get_item_image(container_type &c, const columns_type &columns, size_t row, size_t column) const
			{
				const handle_type &h(get_row(row));
				int idx=-1;
				bool rc=columns.render_element_image_at(column, idx, handle_to_value(h));
				return idx;
			}
			ui_string get_item_text(container_type &c, const columns_type &columns, size_t row, size_t column) const
			{
				const handle_type &h(get_row(row));
				ui_string rcstring;
				bool rc=columns.render_element_at(column, rcstring, handle_to_value(h));
				return rcstring;
			}
			ui_string as_string(container_type &c, const columns_type &columns, const_iterator i) const
			{
				ui_string rcstring;
				bool rc=columns.render_element_at(0, rcstring, handle_to_value(*i));
				return rcstring;
			}

			void refresh_handles(container_type &c) const
			{
				m_handles.resize(c.size());
				sorted_handles_t::iterator n=m_handles.begin();
				for (container_type::iterator i=c.begin(); i!=c.end(); ++i) {
					*n++=i;
				}
				if (m_sort_fnc)
					m_sort_fnc(m_handles.begin(), m_handles.end());
				m_handles_dirty=false;
			}
			bool comparator(const container_type &c, const columns_type &columns, int column_index, const handle_type &left, const handle_type &right)
			{
				return columns.compare(column_index, *left, *right);
			}
			static inline void do_sort(typename sorted_handles_t::iterator start, typename sorted_handles_t::iterator stop, const basic_columns_sorter<column_descriptor, handle_policies_type> &sorting)
			{
				std::sort(start, stop, sorting);
			}
			void set_sort_order(const container_type &c, const columns_type &columns, int column_index)
			{
				m_sorting.push_sort(column_index, columns.at(column_index));
				m_sort_fnc=bind(&container_policies_type::do_sort, _1, _2, boost::ref(m_sorting));
			}
			void clear_sort_order()
			{
				m_sort_fnc.clear();
				m_sorting.clear();
			}
			size_t size(container_type &c) const { return handles(c).size(); }

			const_iterator begin(container_type &c) const { return handles(c).begin(); }
			const_iterator end(container_type &c) const { return handles(c).end(); }

			value_type &at(container_type &c, size_t idx) { return *handles(c).at(idx); }
			const value_type &at(const container_type &c, size_t idx) const { return *handles(c).at(idx); }
			void set_at(container_type &c, size_t idx, const value_type &v) { value_type &old=at(c, idx); old=v; }

			//TODO: implement insert_stable for different container types
			void append(container_type &c, const value_type &v)
			{
				handle_type h=c.insert(c.end(), v);
				if (container_policies_type::insert_stable)
					m_handles.push_back(h);
				else
					m_handles_dirty=true;
			}

			void clear(container_type &c)
			{
				c.clear();
				m_handles.clear();
				m_handles_dirty=false;
			}
			void erase(container_type &c, size_t idx)
			{
				c.erase(handles(c).at(idx));
				if (container_policies_type::erase_stable)
					m_handles.erase(m_handles.begin()+idx);
				else
					m_handles_dirty=true;
			}

			stl_container_policies()
				:m_handles_dirty(true){}
		protected:
			handle_policies_type m_handle_policies;
			mutable bool m_handles_dirty;
			boost::function<void(typename sorted_handles_t::iterator, typename sorted_handles_t::iterator)> m_sort_fnc;
			basic_columns_sorter<column_descriptor, handle_policies_type> m_sorting;
			sorted_handles_t &handles(container_type &c) { if (m_handles_dirty) refresh_handles(c); return m_handles; }
			sorted_handles_t &handles(const container_type &c) const { if (m_handles_dirty) refresh_handles(const_cast<container_type&>(c)); return m_handles; }
			mutable sorted_handles_t m_handles;
			const handle_type &get_row(size_t row) const { return m_handles[row]; }
			const value_type &handle_to_value(const handle_type &h) const { return m_handle_policies.handle_to_value(h); }
		};

		template <typename Value>
		class container_policies<std::vector<Value>, Value >:public stl_container_policies<std::vector<Value> >
		{
		public:
		};

		template <typename Value>
		class container_policies<std::list<Value>, Value>:public stl_container_policies<std::list<Value> >
		{
			typedef stl_container_policies<std::list<Value> > Inherited;
		public:
		};

		
		//------------------------------------------------------------------------------------------------------------------------------------
		
		template <typename Container, typename UIControl, typename ContainerPolicies=container_policies<Container>, typename UIControlPolicies=uicontrol_policies<UIControl> >
		class list_mediator
		{
		public:
			typedef typename ContainerPolicies::container_type container_type;
			typedef typename container_type::value_type value_type;
			typedef typename ContainerPolicies::columns_type columns_type;
			typedef typename UIControlPolicies::uicontrol_type uicontrol_type;

			typedef typename ContainerPolicies::const_iterator const_iterator;
			typedef typename ContainerPolicies::iterator iterator;

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
			void clear_ui()
			{
				set_ui(0);
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

			void sort_by(int column_index)
			{
				if (column_index>=0)
					m_container_policies.set_sort_order(*m_container, m_columns, column_index);
				else
					m_container_policies.clear_sort_order();
				refresh(true);
			}
			litwindow::wstring get_item_text(size_t row, size_t col) const
			{
				return m_container_policies.get_item_text(*m_container, m_columns, row, col);
			}
			int get_item_image(size_t row, size_t col) const
			{
				return m_container_policies.get_item_image(*m_container, m_columns, row, col);
			}
			litwindow::wstring as_string(const_iterator i) const 
			{ 
				return m_container_policies.as_string(*m_container, m_columns, i); 
			}
			size_t get_item_count() const
			{
				return m_container_policies.size(*m_container);
			}

			static const size_t npos = (size_t)-1;
			size_t get_selection_index() const { return m_uicontrol_policies.get_selection_index(m_uicontrol); }
			bool has_selection() const { return get_selection_index()!=npos; }
			void set_selection_index(size_t i) { m_uicontrol_policies.set_selection_index(m_uicontrol, i); }

			//TODO: implement a container-like interface
			//\name STL interface functions
			//@{
			const value_type &at(size_t idx) const { return value_at(idx); }
			void clear() { delete_all_items(); }
			void remove(size_t idx) { m_container_policies.erase(*m_container, idx); set_dirty(); }
			size_t size() const { return get_item_count(); }
			const_iterator begin() const { return m_container_policies.begin(*m_container); }
			const_iterator end() const { return m_container_policies.end(*m_container); }
			//@}

			value_type &value_at(size_t idx) { return m_container_policies.at(*m_container, idx); }
			const value_type &value_at(size_t idx) const { return m_container_policies.at(*m_container, idx); }
			void set_value_at(size_t idx, const value_type &v) { m_container_policies.set_at(*m_container, idx, v); set_dirty(); }

			void delete_selected_item() { remove(get_selection_index()); }
			const value_type &get_selected_item() const { return value_at(get_selection_index()); }
			void modify_selected_item(const value_type &v) { set_value_at(get_selection_index(), v); }
			void delete_all_items() { m_container_policies.clear(*m_container); set_dirty(); }
			void append_item(const value_type &v) { m_container_policies.append(*m_container, v); set_dirty(); }

			template <typename ResultSet, typename Fnc>
			void visit(ResultSet* rc, size_t idx, Fnc f) const
			{
				rc->push_back(f(value_at(idx)));
			}
			template <typename Fnc, typename ResultSet>
			void get_selection(Fnc f, ResultSet &r) const
			{
				r.clear();
				m_uicontrol_policies.for_each_selected(m_uicontrol, bind(&list_mediator::visit<ResultSet, Fnc>, this, &r, _1, f));
			}
			template <typename Fnc>
			void for_each_selected(Fnc f) const
			{
				m_uicontrol_policies.for_each_selected(m_uicontrol, bind(f, _1));
			}

			void refresh() { refresh(true); }
			void refresh(bool force) { if (force) { set_dirty(); /*m_columns.dirty(true);*/ } do_refresh(); }
			~list_mediator()
			{
				set_ui(0);
			}
			list_mediator():m_dirty(false),m_uicontrol(0),m_container(0){}
			list_mediator(container_type &c, uicontrol_type *u)
			{ set_container(c); set_ui(u); }
			list_mediator &set(container_type &c) { set_container(c); return *this; }
			list_mediator &set(uicontrol_type *u) { set_ui(u); return *this; }
		protected:
			columns_type m_columns;
			uicontrol_type *m_uicontrol;
			container_type *m_container;
			ContainerPolicies m_container_policies;
			UIControlPolicies m_uicontrol_policies;
			bool m_dirty;
			void do_refresh()
			{
				if (m_container && m_uicontrol && !m_columns.empty() && dirty()) {
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
