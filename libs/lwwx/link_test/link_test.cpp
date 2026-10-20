#include <litwindow/wx/lwwx.h>

int main(int argc, char **)
{
	// Never executed; only forces the linker to resolve a symbol from lwwx.
	if (argc > 100)
		litwindow::enable_log_to_wxdebug(false);
	return 0;
}
