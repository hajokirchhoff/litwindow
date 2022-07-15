/*
* Copyright 2004-2005, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library
* distribution, file LICENCE.TXT
* $Id: renderer.cpp,v 1.3 2007/04/10 09:35:01 Hajo Kirchhoff Exp $
*/
#include "stdafx.h"
#include "litwindow/dataadapter.h"
#include "litwindow/logging.h"
#include "litwindow/renderer.hpp"
#include <boost/format.hpp>
#include <boost/preprocessor/cat.hpp>
using namespace std;
using namespace litwindow;

namespace litwindow {
	namespace dataadapter {

		namespace {

			template <typename SourceType>
			void render_default(tstring &out, const const_accessor &in, const tstring &format)
			{
				out=boost::str(boost::basic_format<TCHAR>(format) % dynamic_cast_accessor<SourceType>(in).get());
			}
			register_renderer<int, tstring> render_int_r(&render_default<int>, _T("%1%"), renderer<tstring>::get());
			register_renderer<float, tstring> render_float_r(&render_default<float>, _T("%1%"), renderer<tstring>::get());
			register_renderer<double, tstring> render_double_r(&render_default<double>, _T("%1%"), renderer<tstring>::get());

#define DEFAULT_RENDERER(t) \
	register_renderer<t, tstring> BOOST_PP_CAT(renderer_r, __LINE__)(&render_default<t >, _T("%1%"), renderer<tstring>::get())
			DEFAULT_RENDERER(unsigned int);
			DEFAULT_RENDERER(short);
			DEFAULT_RENDERER(unsigned short);
			DEFAULT_RENDERER(char);
			DEFAULT_RENDERER(unsigned char);
			DEFAULT_RENDERER(bool);

			void render_tstring(tstring &out, const const_accessor &in, const tstring &format)
			{
				if (format==_T("%1%"))
					out=in.to_string();
				else
					render_default<tstring>(out, in, format);
			}
			register_renderer<tstring, tstring> render_tstring_r(&render_tstring, _T("%1%"), renderer<tstring>::get());
		};

		renderer<tstring> &get_default_tstring_renderer()
		{
			static renderer<tstring> g_default;
			return g_default;
		}

		namespace {
			inline void default_tstring_renderer_object(tstring &rc, const const_accessor &a, const tstring &format)
			{
				rc=a.to_string();
			}
		};

		renderer<tstring>::render_object_t get_default_tstring_render_object()
		{
			static renderer<tstring>::render_object_t g_default(boost::bind(&default_tstring_renderer_object, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3));
			return g_default;
		}

	};
};
