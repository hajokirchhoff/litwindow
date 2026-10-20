# Compile-only regression test: litwindow::ui::list_mediator<..., wx::VirtualListCtrl>
# must resolve its default uicontrol_policies argument to litwindow::wx::uicontrol_policies,
# regardless of whether ui/list_mediator.hpp or wx/list_mediator.hpp is included first.
cmake_minimum_required(VERSION 3.10)

add_library(lwwx_policies_test OBJECT
	ui_first.cpp
	wx_first.cpp
)
target_link_libraries(lwwx_policies_test PRIVATE lwwx)
set_property(TARGET lwwx_policies_test PROPERTY CXX_STANDARD 20)
