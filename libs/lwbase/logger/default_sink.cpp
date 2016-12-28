/* 
* Copyright 2009, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
*/
#include "stdafx.h"
#if defined(LITWINDOW_LOGGER_EXPORTS) || defined(LWBASE_EXPORTS)
#include "..\..\..\litwindow\logger\sink.hpp"

namespace litwindow {
	namespace logger {
		//template struct global_sink_data<char>;
		//template struct global_sink_data<wchar_t>;

		template global_sink_data<char> LITWINDOW_LOGGER_API &global_sink_data<char>::instance();
		template global_sink_data<wchar_t> LITWINDOW_LOGGER_API &global_sink_data<wchar_t>::instance();
		template basic_tag<char>::container_type LITWINDOW_LOGGER_API &basic_tag<char>::name_container();
		template basic_tag<wchar_t>::index_container_type LITWINDOW_LOGGER_API &basic_tag<wchar_t>::index_container();
	}
}
#endif
