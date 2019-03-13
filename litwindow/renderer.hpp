/* 
 * Copyright 2004-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 */
#ifndef _LWL_RENDERER_
#define _LWL_RENDERER_

#pragma once

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <map>
#include "litwindow/dataadapter.h"

namespace litwindow {

	namespace dataadapter {

		template <typename TargetType>
		struct render_object {
			typedef boost::function<void(TargetType&, const const_accessor &, const tstring &format)> render_fnc_t;
			typedef TargetType target_type_t;
			render_fnc_t m_fnc;
			render_object(render_fnc_t fnc)
				:m_fnc(fnc)
			{
			}
			render_object(void(*fnc)(TargetType&, const const_accessor &, const tstring &format))
				:m_fnc(boost::bind(fnc, _1, _2, _3))
			{
			}
			render_object(){}
		};

		template <typename TargetType>
		class renderer
		{
		public:
			typedef render_object<TargetType> render_object_t;
			typedef std::map<prop_t, std::pair<render_object_t, tstring> > renderer_map_t;

			renderer():m_linked_renderer(0) {}
			renderer(renderer<TargetType> &linked_renderer):m_linked_renderer(&linked_renderer) {}

			void add(prop_t source_type, const render_object_t &r, const tstring &default_format)
			{
				m_objects[source_type]=std::make_pair(r, default_format);
			}
			template <typename SourceType>
			void add(const render_object_t &r, const tstring &default_format)
			{
				add(get_prop_type<SourceType>(), r, default_format);
			}

			void operator()(TargetType &target, const const_accessor &a, const tstring &format)
			{
				std::pair<render_object_t, tstring> r(find_render_object(a));
				r.first.m_fnc(target, a, format.empty() ? r.second : format);
			}
			void operator()(TargetType &target, const const_accessor &a)
			{
				std::pair<render_object_t, tstring> r(find_render_object(a));
				r.first.m_fnc(target, a, r.second);
			}
			TargetType operator()(const const_accessor &a, const tstring &format)
			{
				TargetType rc;
				operator()(rc, a, format);
				return rc;
			}
			bool find_render_object(const const_accessor &a, std::pair<render_object_t, tstring> &rc)
			{
				renderer_map_t::const_iterator i=m_objects.find(a.get_type());
				bool found= i!=m_objects.end();
				if (found)
					rc=i->second;
				else if (m_linked_renderer)
					found=m_linked_renderer->find_render_object(a, rc);
				return found;
			}
			std::pair<render_object_t, tstring> find_render_object(const const_accessor &a)
			{
				std::pair<render_object_t, tstring> rc;
				if (find_render_object(a, rc)==false)
					rc=make_pair(get_default_render_object(), tstring());
				return rc;
			}
			inline render_object_t get_default_render_object();
			static renderer<TargetType> &get();
		protected:
			renderer_map_t m_objects;
			renderer<TargetType> *m_linked_renderer;
		};

		template <typename TargetType>
		struct default_renderer
		{
			renderer<TargetType> &get() const;
		};

		template <typename TargetType>
		renderer<TargetType> &renderer<TargetType>::get()
		{ 
			return default_renderer<TargetType>().get(); 
		}

		template <typename SourceType, typename TargetType=tstring>
		class register_renderer
		{
			typedef render_object<TargetType> RenderObject;
		public:
			register_renderer(const RenderObject &ro, renderer<TargetType>&render=default_renderer<TargetType>().get())
			{
				render.add<SourceType>(ro);
			}
			register_renderer(const RenderObject &ro, const tstring &default_format, renderer<TargetType>&render=default_renderer<TargetType>().get())
			{
				render.add<SourceType>(ro, default_format);
			}
		};

		extern LWBASE_API renderer<tstring>::render_object_t get_default_tstring_render_object();
		template <>
		renderer<tstring>::render_object_t renderer<tstring>::get_default_render_object()
		{
			return get_default_tstring_render_object();
		}

		extern LWBASE_API renderer<tstring> &get_default_tstring_renderer();

		template<>
		struct default_renderer<tstring> {
			renderer<tstring> &get() const
			{
				return get_default_tstring_renderer();
			}
		};
	};

	using namespace dataadapter;
};

#endif
