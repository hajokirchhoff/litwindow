#ifndef lwbase__edit_mediator_h__
#define lwbase__edit_mediator_h__

#include "litwindow/lwbase.hpp"

namespace litwindow { namespace ui {

	template <typename ValueType>
	class basic_edit_mediator
	{
	public:
		typedef ValueType value_type;
		boost::signal<void()> begin_edit;
		boost::signal<void()> end_edit;
		boost::signal<void(bool)> enable_edit;
		virtual void set_value(const value_type &v) = 0;
		virtual void get_value(value_type &v) = 0;

		virtual ~basic_edit_mediator() {}
	};

	template <typename ListMediator>
	class list_edit_mediator:public basic_edit_mediator<typename ListMediator::value_type>
	{
	public:
		typedef ListMediator list_mediator_t;
		typedef typename list_mediator_t::value_type value_type;
		typedef typename list_mediator_t::handle_type handle_type;
		void set_value(const value_type &v);
		void get_value(value_type &v);

		void selection_changed(list_mediator_t *mediator, handle_type selection);
		void selection_cleared(list_mediator_t *mediator);
		void update_selection(list_mediator_t *mediator);

		list_edit_mediator()
			:m_position_valid(false),m_mediator(0){}
	private:
		handle_type m_position;
		bool m_position_valid;
		list_mediator_t *m_mediator;
	};

	
	//------------------------------------------------------------------------------------------------------------------------------------
	template <typename ListMediator>
	void list_edit_mediator<ListMediator>::selection_changed(list_mediator_t *mediator, handle_type selection)
	{
		if (!m_position_valid)
			enable_edit(true);
		m_mediator=mediator;
		m_position_valid=true;
		m_position=selection;
		begin_edit();
	}
	template <typename ListMediator>
	void list_edit_mediator<ListMediator>::selection_cleared(list_mediator_t *mediator)
	{
		if (m_position_valid)
			enable_edit(false);
		m_position_valid=false;
		m_mediator=mediator;
	}
	template <typename ListMediator>
	void list_edit_mediator<ListMediator>::update_selection(list_mediator_t *mediator)
	{
		if (m_position_valid) {
			if (m_mediator==mediator) {
				end_edit();
			}
		}
	}
	template <typename ListMediator>
	void list_edit_mediator<ListMediator>::set_value(const value_type &v)
	{
		if (m_position_valid && m_mediator) {
			m_mediator->value_at_handle(m_position)=v;
			m_mediator->refresh();
		}
	}
	template <typename ListMediator>
	void list_edit_mediator<ListMediator>::get_value(value_type &v)
	{
		if (m_position_valid && m_mediator) {
			v=m_mediator->value_at_handle(m_position);
		}
	}
	
}}
#endif // lwbase__edit_mediator_h__
