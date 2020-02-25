#pragma once

namespace litwindow {
	namespace ui {
		template <typename Container, typename UIControl, typename ContainerPolicies, typename UIControlPolicies >
		inline bool list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::dirty() 
		{
			return m_dirty || m_columns.dirty();
		}

	}
}



template <typename Container, typename UIControl, typename ContainerPolicies/*=container_policies<Container>*/, typename UIControlPolicies/*=uicontrol_policies<UIControl> */>
void litwindow::ui::list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::do_refresh()
{
	if (m_container && m_uicontrol && !m_columns.empty() && dirty()) {
		m_uicontrol_policies.begin_update(m_uicontrol);
		if (m_columns.dirty()) {
			m_uicontrol_policies.refresh_columns(*this, m_uicontrol);
			m_columns.dirty(false);
		}
		m_container_policies.refresh_handles(*m_container, m_uicontrol_policies.get_cache_hint(m_uicontrol));
		m_uicontrol_policies.refresh_rows(*this, m_uicontrol);
		m_uicontrol_policies.end_update(m_uicontrol);
		m_dirty=false;
	}
}

template <typename Container, typename UIControl, typename ContainerPolicies/*=container_policies<Container>*/, typename UIControlPolicies/*=uicontrol_policies<UIControl> */>
void litwindow::ui::list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::set_layout_perspective( const wstring &layout )
{
	if (layout.empty()==false && !boost::algorithm::istarts_with(layout, L"<?xml")) {
		std::wstring previous_layout;
		get_layout_perspective(previous_layout);
		try {
			std::wstringstream in(layout);
			boost::archive::text_wiarchive ar(in);
			ar >> boost::serialization::make_nvp("mediator", *this);
		}
		catch (boost::archive::archive_exception &) {
			// Invalid archive. Ignore layout perspective.
			try {
				std::wstringstream in(previous_layout);
				boost::archive::text_wiarchive ar(in);
				ar >> boost::serialization::make_nvp("mediator", *this);
			}
			catch (...) {
			// catch-all to prevent double exception caused by a programming error. previous_layout really should be serializeable, but
			// perhaps it's not.
			}
		}
		refresh(true);
	}
}


template <typename Container, typename UIControl, typename ContainerPolicies/*=container_policies<Container>*/, typename UIControlPolicies/*=uicontrol_policies<UIControl> */>
void litwindow::ui::list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::get_layout_perspective( wstring &layout )
{
	std::wstringstream out;
	boost::archive::text_woarchive ar(out);
	ar << boost::serialization::make_nvp("mediator", *this);
	layout=out.str();
}

template <typename Container, typename UIControl, typename ContainerPolicies/*=container_policies<Container>*/, typename UIControlPolicies/*=uicontrol_policies<UIControl> */>
template <typename Archive>
void litwindow::ui::list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::serialize( Archive &ar, const unsigned int version )
{
	if (Archive::is_saving()) {
		m_uicontrol_policies.get_columns(*this, m_uicontrol);
	}
	ar & BOOST_SERIALIZATION_NVP(m_columns);
	std::vector<basic_columns_sort_index> sortorder;
	if (Archive::is_loading()) {
		ar & boost::serialization::make_nvp("sort-order", sortorder);
		set_sort_order(sortorder);
	} else {
		sortorder = get_sort_order();
		ar & boost::serialization::make_nvp("sort-order", sortorder);
	}
}

template <typename Container, typename UIControl, typename ContainerPolicies/*=container_policies<Container>*/, typename UIControlPolicies/*=uicontrol_policies<UIControl> */>
void litwindow::ui::list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::toggle_show_column( size_t col )
{
	columns().toggle_show(col);
	refresh();
}
template <typename Container, typename UIControl, typename ContainerPolicies/*=container_policies<Container>*/, typename UIControlPolicies/*=uicontrol_policies<UIControl> */>
std::vector<size_t> litwindow::ui::list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::get_selection()
{
	std::vector<size_t> rc;
	for_each_selected(bind(&list_mediator::visit_index<std::vector<size_t> >, this, &rc, _1));
	return rc;
}

template <typename Container, typename UIControl, typename ContainerPolicies/*=container_policies<Container>*/, typename UIControlPolicies/*=uicontrol_policies<UIControl> */>
typename litwindow::ui::list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::value_type & litwindow::ui::list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::get_selected_item()
{
	if (has_selection()==false) {
		throw std::runtime_error("no item selected");
	}
	return value_at(get_selection_index());
}

template <typename Container, typename UIControl, typename ContainerPolicies/*=container_policies<Container>*/, typename UIControlPolicies/*=uicontrol_policies<UIControl> */>
const typename litwindow::ui::list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::value_type & litwindow::ui::list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::get_selected_item() const
{
	if (has_selection()==false) {
		throw std::runtime_error("no item selected");
	}
	return value_at(get_selection_index());
}

template <typename Container, typename UIControl, typename ContainerPolicies/*=container_policies<Container>*/, typename UIControlPolicies/*=uicontrol_policies<UIControl> */>
size_t litwindow::ui::list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::get_item_count() const
{
	return m_container ? m_container_policies.size(*m_container) : 0;
}

template <typename Container, typename UIControl, typename ContainerPolicies/*=container_policies<Container>*/, typename UIControlPolicies/*=uicontrol_policies<UIControl> */>
std::wstring litwindow::ui::list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::as_string( const_iterator i ) const
{
	return m_container_policies.as_string(*m_container, m_columns, i);
}

template <typename Container, typename UIControl, typename ContainerPolicies/*=container_policies<Container>*/, typename UIControlPolicies/*=uicontrol_policies<UIControl> */>
int litwindow::ui::list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::get_item_image( size_t row, size_t col ) const
{
	return m_container_policies.get_item_image(*m_container, m_columns, row, col);
}

template <typename Container, typename UIControl, typename ContainerPolicies/*=container_policies<Container>*/, typename UIControlPolicies/*=uicontrol_policies<UIControl> */>
void litwindow::ui::list_mediator<Container, UIControl, ContainerPolicies, UIControlPolicies>::set_sort_order( const std::vector<basic_columns_sort_index> &sortorder )
{
	m_container_policies.clear_sort_order();
	for (size_t i=0; i<sortorder.size(); ++i) {
		if (sortorder[i].m_column_index>=0)
			m_container_policies.set_sort_order(*m_container, m_columns, sortorder[i]);
	}
}
