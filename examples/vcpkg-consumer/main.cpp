// Minimal smoke test: proves that a downstream project can find and link
// litwindow purely through vcpkg + find_package(litwindow), without any
// knowledge of litwindow's own source layout.
#include <litwindow/lwbase.hpp>

#ifdef LITWINDOW_CONSUMER_HAS_ODBC
#include <litwindow/odbc/lwodbc.h>
#endif

int main()
{
	return 0;
}
