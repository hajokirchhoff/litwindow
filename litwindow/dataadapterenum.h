#ifndef dataadapterenum_h__
#define dataadapterenum_h__
/*
* Copyright 2009, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library
* distribution, file LICENCE.TXT
* $Id: dataadapter.h,v 1.12 2007/10/26 11:52:59 Hajo Kirchhoff Exp $
*/
#include "litwindow/tstring.hpp"
#include <vector>
#include <limits>
#include "litwindow/dataadapter.h"
namespace litwindow {
    class enum_adapter
    {
    public:
        struct element
        {
            tstring m_name;
            tstring m_description;
            int     m_value;
            bool operator==(const element &r) const { return m_value==r.m_value; }
            bool operator<(const element &r) const { return m_value<r.m_value; }
            element(const tstring::value_type *ptr, int value)
                :m_name(ptr),m_value(value) { }
            element():m_value(std::numeric_limits<int>::min()){}
        private:
            friend class enum_adapter;
            element(int v):m_value(v){}
        };
        static const element &null() { static const element __n; return __n; }
        typedef std::vector<element> elements_t;
        class inserter
        {
            const tstring::value_type *m_name;
        public:
            inserter(const tstring::value_type *e):m_name(e) {}
            void add(enum_adapter *o) const { o->add(m_name); }
        };
        void add(const tstring::value_type *e)
        {
            m_values.push_back(element(e, m_next_value++));
        }
		void add(const element &e)
		{
			m_values.push_back(element(e));
			m_next_value=e.m_value+1;
		}
		enum_adapter operator+(const element &e)
		{
			enum_adapter rc(*this);
			rc.add(e);
			return rc;
		}
        enum_adapter operator+(int next_val)
        {
            m_next_value = next_val;
            return *this;
        }
        enum_adapter operator+(const inserter &r)
        {
            enum_adapter rc(*this);
            r.add(&rc);
            return rc;
        }
        enum_adapter &operator=(const inserter &r)
        {
            m_next_value=0;
            r.add(this);
            return *this;
        }
        enum_adapter():m_next_value(0){}
        enum_adapter(const inserter &r)
        {
            r.add(this); 
        }
        const element &find(int v) const
        {
            elements_t::const_iterator i=std::find(m_values.begin(), m_values.end(), element(v));
            return i==m_values.end() ? null() : *i;
        }
        bool find(const litwindow::tstring &n, int &v) const
        {
            elements_t::const_iterator i;
            for (i=m_values.begin(); i!=m_values.end() && i->m_name!=n; ++i)
                ;
            if (i!=m_values.end()) {
                v=i->m_value;
                return true;
            }
            return false;
        }
        size_t size() const { return m_values.size(); }
        const element &value(size_t index) const
        { 
            return *(m_values.begin()+index);
        }
    protected:
        elements_t m_values;
        int m_next_value;
    };

    template <typename enum_value_type>
    class concrete_enum_adapter:public enum_adapter
    {
    public:
        concrete_enum_adapter(const enum_adapter &e):enum_adapter(e) { }
        litwindow::tstring find(enum_value_type v) const { return enum_adapter::find((int)v).m_name; }
        bool find(const litwindow::tstring &n, enum_value_type &v) const
        {
            int val;
            bool rc=enum_adapter::find(n, val);
            if (rc)
                v=enum_value_type(val);
            return rc;
        }
    };
    class converter_enum_info
    {
    public:
        typedef enum_adapter::element element;
        virtual size_t enum_count() const = 0;
        virtual const element &enum_value(size_t index) const = 0;
    };
    template <typename ENUM_TYPE>
    class converter_enum:public converter<ENUM_TYPE>, public converter_enum_info
    {
    public:
        using converter<ENUM_TYPE>::member;
        converter_enum(const std::string &_type_name, const prop_type_registrar *r)
            :converter/*_value_base*/<ENUM_TYPE>(_type_name, r)
        {}
        typedef ENUM_TYPE value_type;
        typedef typename concrete_enum_adapter<ENUM_TYPE>::element element;
        static const concrete_enum_adapter<ENUM_TYPE> &my_adapter();
        bool is_enum() const { return true; }
        bool is_int() const { return true; }
        int to_int(const schema_entry *propertyAccessInfo, const_prop_ptr member_ptr)
        {
            return int(member(member_ptr));
        }
        void from_int(const schema_entry *propertyAccessInfo, int value, prop_ptr member_ptr)
        {
            member(member_ptr)=value_type(value);
        }
        tstring to_string(const schema_entry *entry, const_prop_ptr member_ptr)
        { 
            return my_adapter().find(member(member_ptr)); 
        }
        size_t from_string(const schema_entry *entry, const tstring &value, prop_ptr member_ptr)
        {
            if (my_adapter().find(value, member(member_ptr))==false)
                throw std::runtime_error("invalid value");
            return sizeof(value_type);
        }
        const converter_enum_info *get_enum_info() const { return this; }
        size_t enum_count() const
        { 
            return my_adapter().size(); 
        }
        const element &enum_value(size_t index) const 
        { 
            return my_adapter().value(index); 
        }
    };
}

#define LWL_IMPLEMENT_ENUM_ds(tp, the_decl_spec, tplist) \
    template <> \
    ::litwindow::prop_type_registrar litwindow::prop_type_object<tp >::____register_prop_t(litwindow::prop_type_object<tp >::get(0)); \
    template <> \
    inline void _FORCE_DLL_EXPORT *::litwindow::prop_type_object<tp >::__return_registrar() { return (void*)&____register_prop_t; } \
    /*template <>    \
the_decl_spec ::litwindow::prop_t     litwindow::prop_type_object<tp >::get(const tp *)*/    \
    the_decl_spec ::litwindow::prop_t     get_prop_type_data_adapter_mechanism(const tp*)    \
{    \
    static litwindow::converter_enum<tp > theConverter(#tp, &litwindow::prop_type_object<tp>::____register_prop_t);    \
    return &theConverter;    \
}\
    const litwindow::concrete_enum_adapter<tp> &litwindow::converter_enum<tp>::my_adapter() \
    {\
    static litwindow::concrete_enum_adapter<tp> adapter=litwindow::enum_adapter()+\
    tplist \
    ; \
    return adapter; \
}

#define LWL_IMPLEMENT_ENUM(tp, tplist) LWL_IMPLEMENT_ENUM_ds(tp, /* */, tplist)

#endif // dataadapterenum_h__
