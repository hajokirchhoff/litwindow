if not exist "%HOMEDRIVE%%HOMEPATH%\user-config.jam" (
copy user-config.jam "%HOMEDRIVE%%HOMEPATH%\user-config.jam"
notepad "%HOMEDRIVE%%HOMEPATH%\user-config.jam"
)
bjam -a -q -j%NUMBER_OF_PROCESSORS% --toolset=msvc-9.0 debug release 
