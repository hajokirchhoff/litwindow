#ifndef list_mediator_h__31080910
#define list_mediator_h__31080910

#include "basic_list_mediator.hpp"

namespace litwindow {
	namespace ui {

		//////////////////////////////////////////////////////////////////////////
		//------------------------------------------------------------------------------------------------------------------------------------
		template <typename Container>
		class container_policies
		{
		public:
			typedef Container container_type;
			typedef typename container_type::value_type value_type;
			typedef typename container_type::iterator handle_type;
			typedef basic_column_descriptor<value_type> column_descriptor;
			typedef basic_columns_adapter<column_descriptor> columns_type;

			void refresh_handles(container_type &c) {}
		};

		template <typename Container, typename UIControl, typename ContainerPolicies=container_policies<Container>, typename UIControlPolicies=uicontrol_policies<UIControl> >
		class list_mediator
		{
		public:
			typedef typename ContainerPolicies::container_type container_type;
			typedef typename ContainerPolicies::columns_type columns_type;
			typedef typename UIControlPolicies::uicontrol_type uicontrol_type;

			void set_ui(uicontrol_type *ctrl) { if (m_uicontrol!=ctrl) { m_uicontrol=ctrl; m_dirty=true; } }
			uicontrol_type *get_ui() const { return m_uicontrol; }

			void set_container(container_type &ctnr) { if (&ctnr!=m_container) { m_container=&ctnr; m_dirty=true; } }
			container_type &get_container() { return *m_container; }

			void columns(const columns_type &c) { m_columns=c; m_dirty=true; }
			columns_type &columns() { return m_columns; }

			void refresh() { do_refresh(); }
			void refresh(bool force) { if (force) m_dirty=true; do_refresh(); }
		protected:
			columns_type m_columns;
			uicontrol_type *m_uicontrol;
			container_type *m_container;
			ContainerPolicies m_container_policies;
			UIControlPolicies m_uicontrol_policies;
			bool m_dirty;
			void do_refresh()
			{
				if (m_dirty) {
					m_uicontrol_policies.begin_update(m_uicontrol);
					m_container_policies.refresh_handles(*m_container);
					m_uicontrol_policies.refresh_columns(*this, m_uicontrol);
					m_uicontrol_policies.end_update(m_uicontrol);
					m_dirty=false;
				}
			}
		};
		
    }

};


#endif // list_mediator_h__31080910
