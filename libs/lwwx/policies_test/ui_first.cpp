#include <litwindow/ui/list_mediator.hpp>
#include <litwindow/wx/list_mediator.hpp>
#include <vector>

struct Row { int value; };

namespace {
	void instantiate(std::vector<Row> &data, litwindow::wx::VirtualListCtrl *ctrl)
	{
		litwindow::ui::list_mediator<std::vector<Row>, litwindow::wx::VirtualListCtrl> m;
		m.set(data);
		m.set_ui(ctrl);
	}
}

void lwwx_policies_test_ui_first(std::vector<Row> &data, litwindow::wx::VirtualListCtrl *ctrl)
{
	instantiate(data, ctrl);
}
