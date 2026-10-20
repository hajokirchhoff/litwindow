#include <litwindow/dataadapter.h>
#include <iostream>

int main()
{
	int value = 0;
	litwindow::accessor a = litwindow::make_accessor(value);
	a.set(42);
	if (value != 42) {
		std::cerr << "accessor::set<int> did not assign the value\n";
		return 1;
	}
	return 0;
}
