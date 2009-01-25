January 25th 2008

How to build:

1. Copy user-config.jam to your home directory (%HOMEDRIVE%%HOMEPATH% under Windows) or append the contents to your own user-config.jam.
2. Modify user-config.jam: change these lines at the beginning of the file
	path-constant BOOST_ROOT : D:/prj/r4000-v4/libs/boost ;
	path-constant WXWIDGETS_ROOT : D:/prj/r4000-v4/libs/wxWidgets ;
   and insert the paths to your boost and wxWidgets installation there.
3. Open a command shell and type
	bjam -q --toolset=msvc-9.0 debug release
   to build the library with Visual Studio 2008. Use a different toolset if you like.
   You can add -j2 if you have two CPU cores to speed up building the library.


Feedback welcome.
Hajo
