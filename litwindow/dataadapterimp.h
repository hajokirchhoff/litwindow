/*
* Copyright 2004-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library
* distribution, file LICENCE.TXT
* $Id: dataadapterimp.h,v 1.12 2008/03/05 17:32:21 Merry\Hajo Kirchhoff Exp $
*/
#ifndef _LITWINDOW_DATAADAPTERIMP_
#define _LITWINDOW_DATAADAPTERIMP_

#ifdef _MSC_VER
// disable warning: 'identifier was truncated to 'number' characters in the debug information'
// 4505: unreferenced local function
#pragma warning(disable:4786 4505)
#endif

#include <algorithm>
#include <vector>
#include <stdexcept>
//!@file
///Macros for defining data adapters
//!@internal non-public declarations and implementation of dataadapter templates.
/**@note A recurring theme in these sources is the broken VC 6.0 template support.
Throughout the code you will find the following construction:
template <class Value>
struct some_struct
{
static returnValue get(Value &v)
{
// do something
}
};

template <class Value>
inline returnValue make_some_struct(Value &v)
{
return some_struct<Value>::get(v);
}

This is a workaround for the broken template function code generation. The code in
some_struct::get could just as well have been put into make_some_struct itself and
the 'some_struct' structure be eliminated. This causes linker problems. VC 6.0 does
not generate proper code for overloaded template functions.
*/
//-----------------------------------------------------------------------------------------------------------//

/** Includes for std::string/char implementations.
@note: All those functions throwing a std::exception have to use std::strings even in UNICODE.
@note: Names of properties are of type char* or std::string. wchar_t and wstring names are not supported.
The reason behind this decision is that the property names are in most cases the names of C++ identifiers.
As of today (2004) no C++ compilers I am aware of support Unicode identifiers. C++ identifier are still
restricted to [_A-Za-z][_A-Za-z0-9]*
Because of that, names are stored as characters. Always. to_string/from_string
*/
#include "./tstring.hpp"

#ifdef _MSC_VER
#pragma warning(push)
// turn off unreferenced formal parameter
#pragma warning(disable:4100)
#endif

#ifndef x_DOC_DEVELOPERS

namespace litwindow {
	//! @defgroup data_adapter_imp Data aggregate Implementation
	//!@{

	class LWBASE_API const_accessor;
	class LWBASE_API accessor;

	//-----------------------------------------------------------------------------------------------------------//
	/** @defgroup data_adapter_schema data aggregate schema
	A Schema is a collection describing the members (properties) of a class.
	@{
	*/
	//#region Schema

	class converter_base;
	class schema_base;

	/// A property type is defined as the pointer to a conversion object accepting this type.
	typedef converter_base *prop_t;
	typedef void *prop_ptr;
	typedef const void *const_prop_ptr;

	/// this structure holds enough bytes storage to store a member pointer
	const size_t _member_pointer_size=sizeof (void (std::exception::*)(void));
	struct _member_pointer_storage
	{
		unsigned char data[_member_pointer_size*2];
		void zero()
		{
			memset(data, 0, sizeof(data));
		}
	};

	///Struct to reserve memory for pointers to get/set methods
	struct getter_setter {
		int compare(const getter_setter &b) const
		{
			return memcmp(this, &b, sizeof (*this));
		}
		_member_pointer_storage getter;
		_member_pointer_storage setter;
	};

	/// Store pointers to get/set methods for a property
	template <class Getter, class Setter>
	struct GetterSetterPointer
	{
		Getter getter;
		Setter setter;
		GetterSetterPointer(Getter _g, Setter _s)
			:getter(_g), setter(_s)
		{}
		static inline const Getter &get_getter(const getter_setter &s)
		{
			return (*(GetterSetterPointer<Getter, Setter>*)&s).getter;
		}
		static inline const Setter &get_setter(const getter_setter &s)
		{
			return (*(GetterSetterPointer<Getter, Setter>*)&s).setter;
		}
	};

	/** Stuff for external (non-member) get/set methods.
	While getter_setter are member functions, ext_getter_setter allow plain, external functions to act as get/set methods.
	*/
	struct ext_getter_setter
	{
		void *getter;
		void *setter;
		int compare(const ext_getter_setter &b) const
		{
			return memcmp(this, &b, sizeof (*this));
		}
	};

	/// construct an ext_getter_setter object from external get/set methods
	template <class GetMethod, class SetMethod>
	inline ext_getter_setter make_ext_getter_setter(GetMethod g, SetMethod s)
	{
		ext_getter_setter rc={reinterpret_cast<void*>(g), reinterpret_cast<void*>(s)};
		return rc;
	}

	/// construct a 'getter_setter' object from a get and set method
	template <class Getter, class Setter>
	getter_setter make_getter_setter(Getter _get, Setter _set)
	{
		GetterSetterPointer<Getter, Setter> values(_get, _set);
		getter_setter rc;
		/*  The following line is a compile time assertion:
		if you receive a compile time error here 'named bit field cannot have zero with',
		it is because the expression (sizeof(p)==sizeof(s)) evaluates to false.
		The reason for that is if the sizeof the member pointer is too large (or small)
		to fit in the allocated memory storage at s.
		To solve this problem, correct the 'const size_t _member_pointer_size = ' line
		above so that _member_pointer_size equals the maximum number of bytes a member pointer
		requires to be stored.
		Note: Usually pointers to virtual members require twice the size than that of pointers to
		non-virtual members.
		*/
		static struct {
			int _member_pointer_storage_too_small_for_pointer: (sizeof (values)<=sizeof(rc));
		} s;
		memcpy(&rc, &values, sizeof(values));
		//rc=*(const getter_setter*)(void*)&values;
		return rc;
	}

	//! This structure holds the information for one member (property) of a class or struct or podt.
	struct schema_entry
	{
		//! Type of the property.
		prop_t m_type;
		//! Determine the type of this schema_entry
		enum entry_type {
			invalid,
			plain_old_type,
			inherited,
			coobject,
			getter_setter_func,
			ext_getter_setter_func
		} m_entry_type;
		//! pointer to a function that casts the parameter to a base this ptr
		typedef void * (*cast_derived_to_base_t)(void* from_derived, void* to_base);
		//! pointer to a function that returns a pointer to a co-object
		typedef void * (*get_co_object_fnc_t)(void *original_object);
		/** Name of the property. @note: only 'char' names are allowed. wchar_t is not supported.
		*/
		const char* m_name;
		//! Name of the class of the property.
		const char* m_class_name;
		union {
			//! Offset in bytes from the beginning of the structure to the property described by this entry. Used for plain old data types.
			size_t m_offset;
			//! Function to cast the this_pointer from the current class to the selected base class. Used for inheritance entries.
			cast_derived_to_base_t  m_cast_derived_to_base;
			get_co_object_fnc_t     m_get_co_object;
			//! Get/Set member pointers to get/set the property. Used for getter_setter_func entries.
			getter_setter       m_getter_setter;
			ext_getter_setter   m_ext_getter_setter;
		};

		/// Accept a plain entry.
		schema_entry(size_t anOffset, prop_t aType, const char* const aName, const char* const aClassName="")
			:m_type(aType)
			,m_entry_type(plain_old_type)
			,m_name(aName)
			,m_class_name(aClassName)
			,m_offset(anOffset)
		{
		}
		/// Create an empty entry.
		schema_entry(const char *propName=0)
			:m_type(0)
			,m_entry_type(invalid)
			,m_name(propName)
			,m_class_name("")
			,m_offset(0)
		{
		}
		/// accept a 'get/set' method
		schema_entry(getter_setter aGetterSetter, prop_t aType, const char * const aName, const char* const aClassName="")
			:m_type(aType)
			,m_entry_type(getter_setter_func)
			,m_name(aName)
			,m_class_name(aClassName)
			,m_getter_setter(aGetterSetter)
		{}
		/// accept an external 'get/set' method
		schema_entry(ext_getter_setter aGetterSetter, prop_t aType, const char * const aName, const char* const aClassName="")
			:m_type(aType)
			,m_entry_type(ext_getter_setter_func)
			,m_name(aName)
			,m_class_name(aClassName)
			,m_ext_getter_setter(aGetterSetter)
		{}
		/// Accept an inheritance entry.
		schema_entry(cast_derived_to_base_t fnc, const prop_t type, const char * const aName)
			:m_type(type)
			,m_entry_type(inherited)
			,m_name(aName)
			,m_class_name(0)
			,m_cast_derived_to_base(fnc)
		{}
		schema_entry(get_co_object_fnc_t fnc, const prop_t type, const char * const aName)
			:m_type(type)
			,m_entry_type(coobject)
			,m_name(aName)
			,m_class_name(0)
			,m_get_co_object(fnc)
		{
		}
#if 0
		bool operator ==(const schema_entry &c) const
		{

			return m_type==c.m_type && m_entry_type==c.m_entry_type &&
				///@todo correct comparison depending on m_entry_type
				m_offset==c.m_offset;
		}

		bool operator < (const schema_entry &c) const
		{
			return m_type<c.m_type ||
				(m_type==c.m_type && (m_entry_type<c.m_entry_type ||
				(m_entry_type==c.m_entry_type && (m_offset<c.m_offset)
				)
				)
				);
		}
#endif
		/// compares two member_ptr/entry combinations.
		/// @returns 0 if they would return the same object (memory location)
		/// -1 if a<b and 1 if b>a
		int compare(const schema_entry &b) const
		{
			if (m_type<b.m_type)
				return -1;
			if (m_type>b.m_type)
				return 1;
			if (m_entry_type<b.m_entry_type)
				return -1;
			if (m_entry_type>b.m_entry_type)
				return 1;
			switch (m_entry_type) {
	case getter_setter_func:
		return m_getter_setter.compare(b.m_getter_setter);
	case ext_getter_setter_func:
		return m_ext_getter_setter.compare(b.m_ext_getter_setter);
		// the other types have already been tested via 'member_ptr'
			}
			return 0;
		}

		bool operator==(const char * aName) const
		{
			return strcmp(m_name, aName)==0;
		}
		/// calculate the memory address of the object that this schema_entry points to.
		/// Input: Base address of the accessor/aggregate
		/// Output: Address of the member
		prop_ptr member_ptr(const void *t) const
		{
			void *aThisPtr=const_cast<void*>(t);
			switch (m_entry_type) {
	case inherited:
		/// calculate base pointer from current address. Call conversion method stored in m_cast_derived_to_base
		aThisPtr=m_cast_derived_to_base(aThisPtr, aThisPtr);
		break;
	case coobject:
		{
			static unsigned loop_detection=0;
			// if this test fails, the coobject creation is caught in an endless loop.
			// most likely case: the constructor of a coobject tries to access
			// a property of the original object. the search for this property
			// tries to create another coobject, since the first coobject has not yet
			// been created. the other coobject constructor again tries to access
			// the property and so on...
			if (++loop_detection==100)
				throw litwindow::lwbase_error("Coobject creation loop! See comments in " __FILE__);
			aThisPtr=m_get_co_object(aThisPtr);
			--loop_detection;
		} break;
	case plain_old_type:
		/// plain old types are simply stored 'm_offset' bytes from the base pointer.
		aThisPtr=reinterpret_cast<unsigned char*>(aThisPtr)+m_offset;
		break;
	case getter_setter_func:
	case ext_getter_setter_func:
		/// will never be called as get/set members use a different way of accessing the pointer
		break;
			}
			return reinterpret_cast<prop_ptr>(aThisPtr);
		}
		bool is_inherited() const
		{
			return m_entry_type==inherited;
		}
		bool is_coobject() const
		{
			return m_entry_type==coobject;
		}
		bool is_nested() const
		{
			return is_inherited() || is_coobject();
		}
	};

	/*! @} */
	//#endregion
	//-----------------------------------------------------------------------------------------------------------//
	/** @defgroup converter Converter for properties
	Property converter are objects that convert a property to other datatypes such as strings, binaries etc...
	@{
	*/

	extern prop_t LWBASE_API get_prop_type_by_name(const char *name);
	/// lookup a type by name.
	inline prop_t LWBASE_API get_prop_type_by_name(const std::string &name)
	{
		return get_prop_type_by_name(name.c_str());
	}
	extern bool LWBASE_API register_prop_type(prop_t type);
	extern bool LWBASE_API unregister_prop_type(prop_t type);

	class prop_type_registrar
	{
		prop_t m_type;
	public:
		prop_type_registrar(prop_t type)
			:m_type(type)
		{
			register_prop_type(type);
		}
		~prop_type_registrar()
		{
			unregister_prop_type(m_type);
		}
	};

	class const_container_iterator_imp_base;
	class container_iterator_imp_base;

	class not_implemented_error:public litwindow::lwbase_error
	{
	public:
		LWBASE_API not_implemented_error(const std::string &method):lwbase_error(std::string("method '")+method+"' not implemented")
		{
		}
	};

    class converter_enum_info;
	/// base class for all converter objects.
	class LWBASE_API converter_base
	{
	public:
		virtual ~converter_base() {}
		virtual std::string get_type_name() const = 0;
		virtual const schema_entry *get_schema_entry() = 0;
		virtual tstring to_string(const schema_entry *propertyAccessInfo, const_prop_ptr member_ptr) = 0;
		virtual size_t from_string(const schema_entry *propertyAccessInfo, const tstring &value, prop_ptr member_ptr) = 0;

		virtual bool is_int() const { return false; }
		virtual int to_int(const schema_entry *propertyAccessInfo, const_prop_ptr member_ptr) { throw not_implemented_error("to_int"); }
		virtual void from_int(const schema_entry *propertyAccessInfo, int value, prop_ptr member_ptr) { throw not_implemented_error("from_int"); }
		virtual bool is_enum() const { return false; }

		virtual bool is_c_vector() const { return false; }

		virtual void from_accessor(const schema_entry *propertyAccessInfo, const const_accessor &a, prop_ptr member_ptr) { throw not_implemented_error("from_accessor"); }
		virtual bool has_copy() const { return false; }
		virtual void copy(const schema_entry *s, const const_accessor &a, prop_ptr member_ptr)
		{
			from_accessor(s, a, member_ptr);
		}
		/*
		virtual bool has_equal_to() const { return false; }
		virtual bool is_equal_to(const schema_entry *s, const const_accessor &a, prop_ptr member_ptr) const = 0;
		virtual bool has_less_than() const { return false; }
		virtual bool is_less_than(const schema_entry *s, const const_accessor &a, prop_ptr member_ptr) const = 0;
		*/
		virtual bool has_schema() const { return false; }
		virtual const schema_base *get_schema() const { return 0; }

		virtual bool is_container() const { return false; }
		virtual bool is_const_container() const { return false; }
		virtual const_container_iterator_imp_base *get_const_begin(const schema_entry *se, const_prop_ptr member_ptr) const { return 0; }
		virtual const_container_iterator_imp_base *get_const_end(const schema_entry *se, const_prop_ptr member_ptr) const { return 0; }
		virtual container_iterator_imp_base *get_begin(const schema_entry *se, prop_ptr member_ptr) const { return 0; }
		virtual container_iterator_imp_base *get_end(const schema_entry *se, prop_ptr member_ptr) const { return 0; }

		// factory methods
		virtual prop_ptr create() const = 0;
		virtual void destroy(const_prop_ptr p) const = 0;
		virtual accessor clone(const schema_entry *e, const_prop_ptr member_ptr) const = 0;
		//virtual size_t ToBinary(const void *element, void *buffer, size_t bufferSize) const;
		//virtual size_t FromBinary(const void *source, size_t sourceSize, void *element, size_t elementSize);

		/// return the size of an object in bytes
		virtual size_t get_sizeof(const schema_entry *e) const = 0;

        /// return enum access
        virtual const converter_enum_info *get_enum_info() const { return 0; }
	};

	/** converter for abstract aggregates. */
	template <class Value >
	class converter_abstract_base:public converter_base
	{
	protected:
		/** helper function to return the name of a method. Uses std::string since
		the return value will be used in std::exception...*/
		inline std::string method_name(const char *m) const
		{
			return std::string("converter<"+type_name+">::"+m);
		}
		std::string type_name;
		const prop_type_registrar *registrar;
	public:
		converter_abstract_base(const std::string &_type_name, const prop_type_registrar *r)
			:type_name(_type_name), registrar(r)
		{}
		const prop_type_registrar *get_registrar() const { return registrar; }
		std::string get_type_name() const { return type_name; }
		virtual bool has_copy() const { return false; }
		//virtual void get_value(Value &v, const schema_entry *e, const_prop_ptr member_ptr) const = 0;
		//virtual const Value *get_ptr(const schema_entry *e, const_prop_ptr member_ptr) const = 0;
		//virtual Value *get_ptr(const schema_entry *e, prop_ptr member_ptr) const = 0;
		//virtual void set_value(const Value &v, const schema_entry *e, prop_ptr member_ptr) = 0;
		//virtual void from_accessor(const schema_entry *e, const const_accessor &a, prop_ptr member_ptr)
		//{
		//    typed_const_accessor<Value> v=dynamic_cast_accessor<Value>(a);
		//    if (!v.is_valid())
		//        throw litwindow::lwbase_error("type mismatch");
		//     set_value(v.get(), e, member_ptr);
		//}

		virtual bool is_int() const
		{
			return false;
		}
		virtual int to_int(const schema_entry *propertyAccessInfo, const_prop_ptr member_ptr) { throw not_implemented_error("to_int"); }
		virtual void from_int(const schema_entry *propertyAccessInfo, int value, prop_ptr member_ptr) { throw not_implemented_error("from_int"); }

		virtual const schema_entry *get_schema_entry()
		{
			static schema_entry the_entry(size_t(0), this, "this");
			return &the_entry;
		}

		virtual prop_ptr create() const
		{
			// cannot create abstract objects
			throw not_implemented_error("create (abstract class!)");
		}
		/// return the size of an object in bytes
		virtual size_t get_sizeof(const schema_entry *e) const
		{
			throw not_implemented_error("get_sizeof (abstract class!)");
		}

		accessor clone(const schema_entry *e, const_prop_ptr member_ptr) const;

		virtual void destroy(const_prop_ptr p) const
		{
			throw not_implemented_error("destroy (abstract class!)");
		}

		virtual tstring to_string(const schema_entry *propertyAccessInfo, const_prop_ptr member_ptr)
		{
			throw not_implemented_error("to_string");
		}
		virtual size_t from_string(const schema_entry *propertyAccessInfo, const tstring &value, prop_ptr member_ptr)
		{
			throw not_implemented_error("from_string");
		}

		virtual bool has_schema() const { return false; }
		virtual const schema_base *get_schema() const { return 0; }
	};

	/** Basic value characteristics, determine if it can be copied, compared etc...
	@todo finish this. Currently its unused and will always return false on compare.
	*/
	template <class Value>
	class value_traits_t
	{
	public:
		bool has_copy() const { return true; }
		void copy(Value &to, const Value &from) const
		{
			from=to;
		}
		bool has_equal_to() const { return true; }
		bool is_equal_to(const Value &one, const Value &two) const
		{
			//            return one==two;
			return false;
		}
		bool has_less_than() const { return true; }
		bool is_less_than(const Value &one, const Value &two) const
		{
			//            return one < two;
			return false;
		}
	};

	/** this class represents a converter with 'get/set' methods for getting/setting the value directly from another value&
	*/
	template <class Value >
	class converter_value_base:public converter_base
	{
	protected:
		/** helper function to return the name of a method. Uses std::string since
		the return value will be used in std::exception...*/
		inline std::string method_name(const char *m) const
		{
			return std::string("converter<"+type_name+">::"+m);
		}
		std::string type_name;
		const prop_type_registrar *registrar;

	public:
		converter_value_base(const std::string &_type_name, const prop_type_registrar *r)
			:type_name(_type_name), registrar(r)
		{}
		//const prop_type_registrar *get_registrar() const { return registrar; }
		std::string get_type_name() const { return type_name; }
		virtual bool has_copy() const { return value_traits_t<Value>().has_copy(); }
		virtual void get_value(Value &v, const schema_entry *e, const_prop_ptr member_ptr) const = 0;
		virtual const Value *get_ptr(const schema_entry *e, const_prop_ptr member_ptr) const = 0;
		virtual size_t get_sizeof(const schema_entry *e) const = 0;
		virtual Value *get_ptr(const schema_entry *e, prop_ptr member_ptr) const = 0;
		virtual void set_value(const Value &v, const schema_entry *e, prop_ptr member_ptr) = 0;
		virtual void from_accessor(const schema_entry *e, const const_accessor &a, prop_ptr member_ptr);

		virtual bool is_int() const
		{
			return false;
		}
		virtual int to_int(const schema_entry *propertyAccessInfo, const_prop_ptr member_ptr) { throw not_implemented_error("to_int"); }
		virtual void from_int(const schema_entry *propertyAccessInfo, int value, prop_ptr member_ptr) { throw not_implemented_error("from_int"); }

		virtual const schema_entry *get_schema_entry()
		{
			static schema_entry the_entry(size_t(0u), this, "this");
			return &the_entry;
		}

		virtual prop_ptr create() const
		{
			return new Value;
		}

		virtual accessor clone(const schema_entry *e, const_prop_ptr member_ptr) const;

		virtual void destroy(const_prop_ptr p) const
		{
			delete static_cast<Value*>(const_cast<void*>(p));
		}
	};

	template <>
	inline bool converter_value_base<int>::is_int() const
	{
		return true;
	}

	template <>
	int converter_value_base<int>::to_int(const schema_entry *propertyAccessInfo, const_prop_ptr member_ptr)
	{
		int v;
		get_value(v, propertyAccessInfo, member_ptr);
		return v;
	}

	template <>
	void converter_value_base<int>::from_int(const schema_entry *e, int value, prop_ptr member_ptr)
	{
		set_value(value, e, member_ptr);
	}
	/** Used by functions that are not implemented yet to throw a lwbase_error. @note: must use std::string
	not tstring here because this is what lwbase_error wants and tstring could be wstring. */
	void LWBASE_API not_implemented(const std::string &what);

	/*! @internal
	The converter class is a template that needs to be specialized for each concrete type.
	Part of this specialization is done through the generic access templates to_string/to_int. See above.

	@note: A very important note on inheriting from converter<Value>! @b DON'T inherit from converter<Value>, if
	Value is a type where you have some partial specialization defined, UNLESS the partial specialization is
	contained in the same compilation unit where you define the inheritance. Example: DON'T inherit from
	converter<int>. The litwindow library includes partial specialization for converter<int>::to_string. If you
	inherit from converter<int>, the MS compiler will instantiate converter<int> and override the partial
	specialization methods that are included in dataadapterimp.cpp. The effect is that if you inherit from
	converter<int>, you have to redefine converter<int>::to_string and other partial specialization methods,
	since the existing partial specialization will be lost. The same applies, if you include
	LWL_IMPLEMENT_ACCESSOR(yourtype) in your source code somewhere and then inherit from converter<yourtype>.
	The only place where you may do this safely is in the same compilation unit where you included the
	LWL_IMPLEMENT_ACCESSOR(yourtype) macro.

	This behaviour is also the reason why converter_with_getset (see below) does not inherit from converter,
	although it would have been very convenient.
	*/
	template <class Value>
	class converter:public converter_value_base<Value>
	{
		typedef converter_value_base<Value> inherited;

	protected:
		//! cast the untyped pointer into a pointer to 'Value'
		inline Value &member(prop_ptr member_ptr) const
		{
			return *static_cast<Value*>(member_ptr);
		}
		inline const Value &member(const_prop_ptr member_ptr) const
		{
			return *static_cast<const Value*>(member_ptr);
		}
	public:
		converter(const std::string &_type_name, const prop_type_registrar *r)
			:converter_value_base<Value>(_type_name, r)
		{}
		virtual void get_value(Value &v, const schema_entry *, const_prop_ptr member_ptr) const
		{
			v=member(member_ptr);
		}
		virtual const Value *get_ptr(const schema_entry *, const_prop_ptr member_ptr) const
		{
			return &member(member_ptr);
		}
		virtual size_t get_sizeof(const schema_entry *) const
		{
			return sizeof(Value);
		}
		virtual Value *get_ptr(const schema_entry *, prop_ptr member_ptr) const
		{
			return &member(member_ptr);
		}
		virtual void set_value(const Value &v, const schema_entry *e, prop_ptr member_ptr)
		{
			member(member_ptr)=v;
		}
		bool is_int() const
		{
			return false;
		}
		int to_int(const Value &v)
		{
			not_implemented(inherited::method_name("to_int"));
			return 0;
		}
		int to_int(const schema_entry *propertyAccessInfo, const_prop_ptr member_ptr)
		{
			return to_int(member(member_ptr));
		}
		void from_int(int newValue, Value &v)
		{
			not_implemented(inherited::method_name("from_int"));
		}
		void from_int(const schema_entry *propertyAccessInfo, int value, prop_ptr member_ptr)
		{
			from_int(value, member(member_ptr));
		}

		bool is_enum() const { return false; }
		//std::vector<tstring> get_enum_values() const
		//{
		//    not_implemented(inherited::method_name("get_enum_values"));
		//    return std::vector<tstring>();
		//}

		tstring to_string(const schema_entry *entry, const_prop_ptr member_ptr)
		{
			return to_string(member(member_ptr));
		}
		size_t from_string(const tstring &newValue, Value &v)
		{
			not_implemented(inherited::method_name("from_string"));
			return 0;
		}
		size_t from_string(const schema_entry *entry, const tstring &value, prop_ptr member_ptr)
		{
			return from_string(value, member(member_ptr));
		}

		bool has_schema() const { return false; }
		const schema_base *get_schema() const { return 0; }
	private:
		/** Convert the value @p v to a std::string.
		@note THIS IS NOT A VIRTUAL FUNCTION! If you derive from converter and must override 'to_string',
		override to_string(const schema_entry*, const_prop_ptr) instead.
		This function is meant to be "overriden" with template specialization.
		*/
		tstring to_string(const Value &v)
		{
			not_implemented(inherited::method_name("to_string"));
			return tstring();
		}
	};

	//-----------------------------------------------------------------------------------------------------------//
	/** Base type object for old style c vector [] data types.
	*/
	template <typename ELEMENT>
	class c_vector_type_base:public converter<ELEMENT>
	{
		typedef converter<ELEMENT> inherited;
	public:
		c_vector_type_base(const std::string &name, const prop_type_registrar *r)
			:converter<ELEMENT>(name, r)
		{
		}
		tstring to_string(const schema_entry *entry, const_prop_ptr member_ptr)
		{
			throw not_implemented(inherited::method_name("to_string"));
			return tstring();
		}
		size_t from_string(const schema_entry *, const tstring &, prop_ptr)
		{
			throw not_implemented(inherited::method_name("from_string"));
			return tstring();
		}
		bool is_c_vector() const { return true; }
	};

	/** Explicit converter::to_string specialization for c_str() types.
	*/
	template <>
	tstring c_vector_type_base<TCHAR>::to_string(const schema_entry *e, const_prop_ptr member_ptr)
	{
		const TCHAR *p=reinterpret_cast<const TCHAR*>(member_ptr);
		return tstring(p);
	}

	/** Explicit converter::from_string specialization for c_str() types.
	*/
	template <>
	size_t c_vector_type_base<TCHAR>::from_string(const schema_entry *e, const tstring &value, prop_ptr member_ptr)
	{
		if (value.length()>=get_sizeof(e)/sizeof(TCHAR))
			throw std::out_of_range(method_name("from_string")+ " input too long");
		TCHAR *p=reinterpret_cast<TCHAR *>(member_ptr);
		_tcscpy(p, value.c_str());
		return value.length()*sizeof(TCHAR);
	}

	/** Concrete type object for old style c vector [] data types including the length of the vector.
	*/
	template <size_t SIZE, typename ELEMENT>
	class c_vector_type:public c_vector_type_base<ELEMENT>
	{
	public:
		c_vector_type(const std::string &name, const prop_type_registrar *r)
			:c_vector_type_base<ELEMENT>(name, r)
		{
		}
		size_t get_sizeof(const schema_entry *) const
		{
			return sizeof(ELEMENT)*SIZE;
		}
	};

	//-----------------------------------------------------------------------------------------------------------//
	//-----------------------------------------------------------------------------------------------------------//
	/** Get a fitting converter object for objects of class @p Value.
	@returns A pointer to a static instance of a converter<Value> object.
	*/

	/** MSVC6 workaround struct to allow get_prop_type. See comment at the top of dataadapterimp.h for explanation.
	*/
	//extern ::litwindow::prop_t get_prop_type_data_adapter_mechanism(const tp*);
	template <class Value>
	struct prop_type_object
	{
		static prop_t get(const Value *t=0)
		{ 
			extern prop_t get_prop_type_data_adapter_mechanism(const Value *);
			return get_prop_type_data_adapter_mechanism(t); 
		}
		static prop_type_registrar ____register_prop_t;
		void _FORCE_DLL_EXPORT *__return_registrar();
	};

#if LITWINDOW_USES_RTTI_LOOKUP==1
	template <class Value>
	inline prop_t get_prop_type(const Value *t=0)
	{
		return get_prop_type_by_name(typeid(Value).name());
	}
#else
	template <typename Value>
	inline prop_t get_lwl_prop_type(const Value *t=0);

	template <class Value>
	inline prop_t get_prop_type(const Value *t=0)
	{
		return prop_type_object<Value>::get(t);
	}
#endif

	/** MSVC6 workaround struct to allow get_c_vector_prop_type. See comment at the top of dataadapterimp.h for explanation.
	*/
	template <size_t SIZE, typename ELEMENT>
	struct c_vector_type_converter_object
	{
		static prop_t get()
		{
			static c_vector_type<SIZE, ELEMENT> theType(typeid(ELEMENT).name(), 0);
			return &theType;
		}
	};

	/** Get a type object for the old style c vector [] data type.
	*/
	template <size_t SIZE, typename ELEMENT>
	prop_t get_c_vector_prop_type(const ELEMENT *t=0)
	{
		return c_vector_type_converter_object<SIZE, ELEMENT>::get();
	}

	/*! Get a schema_entry for objects of class @p Value.
	@returns A const reference to a static instance of a schema_entry object.
	*/
	template <class Value>
	struct prop_schema_entry
	{
		static const schema_entry & get(const Value *t=0)
		{
			return *get_prop_type<Value>(t)->get_schema_entry();
		}
	};

	template <class Value>
	const schema_entry &get_schema_entry(const Value &v)
	{
		return prop_schema_entry<Value>::get(&v);
	}

	template <class Container>
	struct LWBASE_DLL_EXPORT prop_type_container
	{
		static prop_t get(const Container *t=0);
	};

	template <class Container>
	prop_t get_prop_container_type(const Container *t=0)
	{
		return prop_type_container<Container>::get(t);
	}
	/*! @} */

	/*! @addtogroup schema
	@{
	*/
	/*! Objects of this template class hold the actual schema for the classes. */
	template <class Value>
	class schema
	{
		static const char *sm_class_name;
		typedef Value PROPCLASS;
		static schema_base LWBASE_DLL_EXPORT *_init_schema();
	public:
		static LWBASE_DLL_EXPORT const schema_base &get_schema();
	};

	class concrete_factory_base;

	/*! The schema_base container holds the information about all properties of a class.
	This container must preserve the order in which the elements where inserted. The order
	may be important for load/save functions using the properties.
	@note These objects are never constructed directly. schema<Value> constructs objects
	of this class and returns pointers to it via get_schema()
	*/
	class schema_base
	{
		//friend class schema;
		typedef std::vector<schema_entry> schema_vector;
		mutable concrete_factory_base *the_factory;
		/// schema hold the schema_entries for this aggregate only. Inherited members are not included.
		schema_vector this_schema;
		schema_entry this_schema_entry;
	public:
		schema_base(const schema_entry &e):this_schema_entry(e),classname(0),the_factory(0) {}
		~schema_base()
		{
			free_strings();
		}
		bool operator==(const schema_base &b) const
		{
			return this==&b || class_name()==b.class_name();
		}
		bool operator!=(const schema_base &b) const
		{
			return !operator==(b);
		}
		void LWBASE_API init(const schema_entry *staticArray, const char *aClassName);

		typedef schema_vector::const_iterator const_iterator;
		const_iterator begin() const { return this_schema.begin(); }
		const_iterator end() const { return this_schema.end(); }
		size_t size() const { return this_schema.size(); }
	private:
		/// The name of the class for which the schema is stored.
		const char *classname;
		void LWBASE_API free_strings();
		schema_base(const schema_base&);    // cannot copy this class
		void operator=(const schema_base&);
	public:
		const std::string class_name() const { return classname; }
		const const_iterator find(const char *name) const
		{
			const_iterator rc=std::find(begin(), end(), name);
			return rc;
		}
		concrete_factory_base *get_factory() const
		{
			return the_factory;
		}
		void set_factory(concrete_factory_base *new_factory) const
		{
			the_factory=new_factory;
		}
		const schema_entry &get_this_schema_entry() const
		{
			return this_schema_entry;
		}
	};

	template <class Value>
	inline const schema_base &get_schema(const Value &)
	{
		return schema<Value>::get_schema();
	}

	/*! @} */


	template <class derived, class base>
	class cast_this_to_base_class
	{
	public:
		static void* cast_this_to_base(derived *d, base *b)
		{
			// this will fail to compile if b is not base of d
			return static_cast<base*>(d);
		}
	};


	inline bool is_type_alias(prop_t one, prop_t two)
	{
		return one==two || one->get_type_name()==two->get_type_name();
	}
	template <typename Value>
	inline bool is_type(prop_t type)
	{
		return is_type_alias(get_prop_type<Value>(), type);
	}

	//!@}


	//!@}


};

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//

#ifndef _HERE_FOLLOW_LITWINDOW_MACRO_DEFINITIONS
/** \addtogroup data_adapter_reference
@{
*/

//!@name Helper Macros
//!@{
/** \internal
*/
#define IMPLEMENT_ADAPTER_AGGREGATE_NO_COPY(classname) \
	IMPLEMENT_ADAPTER_AGGREGATE(classname) \
	template <>    \
	void ::litwindow::converter<classname >::get_value(classname &, const schema_entry*, const_prop_ptr) const    \
{    \
	not_implemented("get_value");    \
}    \
	template <>    \
	void ::litwindow::converter<classname >::set_value(const classname &, const schema_entry*, prop_ptr)    \
{    \
	not_implemented("set_value");    \
}   \
	template <> \
	void ::litwindow::converter_value_base<classname >::from_accessor(const litwindow::schema_entry*, const const_accessor&, prop_ptr) \
{ \
	not_implemented("from_accessor"); \
} \
	template <> \
	bool ::litwindow::converter_value_base<classname >::has_copy() const { return false; }
/// internal macro

/** \internal
*/
#define IMPLEMENT_ADAPTER_AGGREGATE_ABSTRACT(classname) \
	IMPLEMENT_ADAPTER_TYPE_ABSTRACT(classname) \
	template <> \
	bool ::litwindow::converter_abstract_base<classname >::has_schema() const { return true; } \
	template <> \
	const ::litwindow::schema_base *litwindow::converter_abstract_base<classname >::get_schema() const { return &litwindow::schema<classname >::get_schema(); }

/** \internal
*/
#define IMPLEMENT_ADAPTER_AGGREGATE(classname) \
	LWL_IMPLEMENT_ACCESSOR(classname) \
	template <> \
	bool ::litwindow::converter<classname >::has_schema() const { return true; } \
	template <> \
	const ::litwindow::schema_base *litwindow::converter<classname >::get_schema() const { return &litwindow::schema<classname >::get_schema(); }

/** \internal
*/
#define BEGIN_ADAPTER_AGGREGATE(classname)    \
	template <>    \
	const char *::litwindow::schema<classname >::sm_class_name=#classname;    \
	template <> \
	const ::litwindow::schema_base LWBASE_DLL_EXPORT &litwindow::schema<classname >::get_schema() \
{   \
	static schema_base *schema_ptr=_init_schema(); \
	return *schema_ptr; \
}   \
	template <>    \
	::litwindow::schema_base *litwindow::schema<classname >::_init_schema()    \
{    \
	static schema_base _schema_data(prop_schema_entry<classname >::get(0));    \
	static schema_entry entries[]={

/// internal macro
#define BEGIN_ADAPTER_AGGREGATE_ABSTRACT(classname) \
	BEGIN_ADAPTER_AGGREGATE(classname)

//! \internal This macro is required to implement the neccessary logic so that a type
//! is recognized by the litwindow mechanism.
#define IMPLEMENT_ADAPTER_TYPE_ds(tp, the_decl_spec)    \
	template <> \
	::litwindow::prop_type_registrar litwindow::prop_type_object<tp >::____register_prop_t(litwindow::prop_type_object<tp >::get(0)); \
	template <> \
	inline void _FORCE_DLL_EXPORT *::litwindow::prop_type_object<tp >::__return_registrar() { return (void*)&____register_prop_t; } \
	/*template <>    \
	the_decl_spec ::litwindow::prop_t     litwindow::prop_type_object<tp >::get(const tp *)*/    \
	the_decl_spec ::litwindow::prop_t     get_prop_type_data_adapter_mechanism(const tp*)    \
{    \
	static litwindow::converter<tp > theConverter(#tp, &litwindow::prop_type_object<tp>::____register_prop_t);    \
	return &theConverter;    \
}

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
//! declare a data adapter for a type.
//! Used to export a data adapter from a DLL. Not neccessary when working with static libraries.
#define LWL_DECLARE_ACCESSOR(tp, the_decl_spec)    \
	extern the_decl_spec ::litwindow::prop_t get_prop_type_data_adapter_mechanism(const tp*); \
	/*template <> ::litwindow::prop_t litwindow::prop_type_object<tp >::get(const tp*) { return get_prop_type_data_adapter_mechanism(tp); }*/

//! implement the data adapter for a type.
#define LWL_IMPLEMENT_ACCESSOR(tp) \
	IMPLEMENT_ADAPTER_TYPE_ds(tp, LWBASE_DLL_EXPORT)

#define _NOTHING

#define LWL_IMPLEMENT_INLINE_ACCESSOR(tp) \
	IMPLEMENT_ADAPTER_TYPE_ds(tp, inline)

#define LWL_IMPLEMENT_DLL_ACCESSOR(tp, decl_spec) \
	IMPLEMENT_ADAPTER_TYPE_ds(tp, decl_spec)

#define IMPLEMENT_ADAPTER_TYPE_ABSTRACT(tp)    \
	template <> \
	::litwindow::prop_type_registrar litwindow::prop_type_object<tp >::____register_prop_t(litwindow::prop_type_object<tp >::get(0)); \
	template <> \
	void _FORCE_DLL_EXPORT *::litwindow::prop_type_object<tp >::__return_registrar() { return (void*)&____register_prop_t; } \
	LWBASE_DLL_EXPORT ::litwindow::prop_t     get_prop_type_data_adapter_mechanism(const tp*)    \
	{    \
		static litwindow::converter_abstract_base<tp > theConverter(#tp, &litwindow::prop_type_object<tp>::____register_prop_t);    \
		return &theConverter;    \
	}\
	/*template <>    \
	LWBASE_DLL_EXPORT ::litwindow::prop_t     litwindow::prop_type_object<tp >::get(const tp *)    \
{    \
	static litwindow::converter_abstract_base<tp > theConverter(#tp, &____register_prop_t);    \
	return &theConverter;    \
}*/

//-----------------------------------------------------------------------------------------------------------//
/// Add the neccessary typedefs and functions to a class to make its members accessible via litwindow.
#define LWL_DECLARE_AGGREGATE(classname)   \
	friend class litwindow::schema<classname >;    \
	\
	operator litwindow::const_aggregate() const { return litwindow::make_const_aggregate(*this); }    \
	operator litwindow::aggregate() { return litwindow::make_aggregate(*this); }

#define LWL_BEGIN_AGGREGATE(classname) \
	IMPLEMENT_ADAPTER_AGGREGATE(classname) \
	BEGIN_ADAPTER_AGGREGATE(classname)

#define LWL_BEGIN_AGGREGATE_NO_COPY(classname) \
	IMPLEMENT_ADAPTER_AGGREGATE_NO_COPY(classname) \
	BEGIN_ADAPTER_AGGREGATE(classname)

#define LWL_BEGIN_AGGREGATE_ABSTRACT(classname) \
	IMPLEMENT_ADAPTER_AGGREGATE_ABSTRACT(classname) \
	BEGIN_ADAPTER_AGGREGATE_ABSTRACT(classname)

#define LWL_END_AGGREGATE()    \
	litwindow::schema_entry(0)    \
};    \
	_schema_data.init(entries, sm_class_name);    \
	return (&_schema_data);    \
}

#define PROPVARIABLE(variable)  \
	(((PROPCLASS*)0)->variable)

#define PROPTYPE(variable) \
	::litwindow::get_prop_type(&PROPVARIABLE(variable))

#define PROPENTRY(offset, type, name)    \
	::litwindow::schema_entry(offset, type, name),

/** Define a schema entry for a member variable. */
#define PROP(variable) \
	PROPENTRY(offsetof(PROPCLASS, variable), PROPTYPE(variable), #variable)

/** Define a schema entry for an old style C std::string (null terminated char*).
This macro defines a null terminated C std::string as a property. @note std::string/wstring is strongly preferred to char[].
Use this macro only for older struct/class where you cannot change the struct/class definition. */
#define PROP_CSTR(variable) \
	litwindow::schema_entry(offsetof(PROPCLASS, variable), litwindow::get_c_vector_prop_type<sizeof(PROPVARIABLE(variable))/sizeof(PROPVARIABLE(variable[0]))>(&PROPVARIABLE(variable[0])), #variable),

#define PROP_rename(variable, name) \
	PROPENTRY(offsetof(PROPCLASS, variable), PROPTYPE(variable), #name)

#define PROP_RW_WITH_SIGNATURE(type, get_signature, get_method, set_signature, set_method, name) \
	::litwindow::schema_entry(litwindow::make_getter_setter<get_signature, set_signature >(&get_method, &set_method), litwindow::get_prop_type_getter_setter<type, PROPCLASS, get_signature, set_signature >(&get_method, &set_method), name),

/** Define a schema entry for a get/set property access method pair. */
#define PROP_RW_FUNC(type, get_method, set_method, name) \
	::litwindow::schema_entry(litwindow::make_getter_setter(&PROPCLASS::get_method, &PROPCLASS::set_method), litwindow::get_prop_type_getter_setter<type, PROPCLASS >(&PROPCLASS::get_method, &PROPCLASS::set_method), name),

//#define PROP_R_FUNC(type, get_method, name) \
//    litwindow::schema_entry(litwindow::make_getter_setter(PROPCLASS::get_method, (PROPCLASS::*)0), litwindow::get_prop_type_getter_setter<type, PROPCLASS>(PROPCLASS::get_method, 0), name),

#define PROP_W_FUNC(type, set_method, name) \
	::litwindow::schema_entry(litwindow::make_getter_setter<void (PROPCLASS::*)(void)const>(0, PROPCLASS::set_method), litwindow::get_prop_type_getter_setter<type, PROPCLASS >(0, PROPCLASS::set_method), name),

#define PROP_getset(type, name) \
	PROP_RW_FUNC(type, get##name, set##name, #name)

#define PROP_get_set_(type, name) \
	PROP_RW_FUNC(type, get_##name, set_##name, #name)

#define PROP_GetSet(type, name) \
	PROP_RW_FUNC(type, Get##name, Set##name, #name)

#define PROP_Get(type, name) \
	PROP_R_FUNC(type, Get##name, #name)

#define PROP_Set(type, name) \
	PROP_W_FUNC(type, Set##name, #name)

#define PROP_RW_EXT(type, get_method, set_method, name) \
	::litwindow::schema_entry(litwindow::make_ext_getter_setter(get_method, set_method), litwindow::get_prop_type_ext_getter_setter<type, PROPCLASS >(get_method, set_method), name),

//    #define PROP_A(variable) \
//        litwindow::schema_entry(offsetof(PROPCLASS, variable), &litwindow::get_schema(((PROPCLASS*)0)->variable), #variable),

/** Define a schema entry for a container that conforms to the STL interface. */
#define PROP_C(variable, container) \
	::litwindow::schema_entry(offsetof(PROPCLASS, variable), /*litwindow::prop_type_container<container>::get(&((PROPCLASS*)0)->variable)*/PROPTYPE(variable), #variable),

/** Define a schema entry for a base class. */
#define PROP_I(base) \
	::litwindow::schema_entry(&(litwindow::schema_entry::cast_derived_to_base_t)litwindow::cast_this_to_base_class<PROPCLASS, base >::cast_this_to_base, litwindow::get_prop_type<base >(), #base),

/**@addgroup extensionobjects extension objects
An extension object - or co-object - is an object that implements an extension to the original object.
*/
/// define a 'co' object, an extension object for the object
#define PROP_CO(base, fnc) \
	::litwindow::schema_entry((litwindow::schema_entry::get_co_object_fnc_t)fnc, litwindow::get_prop_type<base >(), #base),

//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------------------------------//

// compatibility with previous versions
#define DECLARE_ADAPTER_TYPE              LWL_DECLARE_ACCESSOR
#define IMPLEMENT_ADAPTER_TYPE      LWL_IMPLEMENT_ACCESSOR
#define IMPLEMENT_INLINE_ADAPTER_TYPE           LWL_IMPLEMENT_INLINE_ACCESSOR

#define DECLARE_ADAPTER                               LWL_DECLARE_AGGREGATE
#define BEGIN_ADAPTER                                       LWL_BEGIN_AGGREGATE
#define BEGIN_ADAPTER_NO_COPY       LWL_BEGIN_AGGREGATE_NO_COPY
#define BEGIN_ADAPTER_ABSTRACT      LWL_BEGIN_AGGREGATE_ABSTRACT
#define END_ADAPTER                                               LWL_END_AGGREGATE
//@}

//@}


LWL_DECLARE_ACCESSOR(int, LWBASE_API)
LWL_DECLARE_ACCESSOR(short, LWBASE_API)
LWL_DECLARE_ACCESSOR(float, LWBASE_API)
LWL_DECLARE_ACCESSOR(bool, LWBASE_API)
LWL_DECLARE_ACCESSOR(long, LWBASE_API)
LWL_DECLARE_ACCESSOR(double, LWBASE_API)
LWL_DECLARE_ACCESSOR(char, LWBASE_API)
LWL_DECLARE_ACCESSOR(unsigned int, LWBASE_API)
LWL_DECLARE_ACCESSOR(unsigned char, LWBASE_API)
LWL_DECLARE_ACCESSOR(unsigned long, LWBASE_API)
LWL_DECLARE_ACCESSOR(litwindow::tstring, LWBASE_API)
LWL_DECLARE_ACCESSOR(unsigned short, LWBASE_API)
LWL_DECLARE_ACCESSOR(litwindow::accessor, LWBASE_API)
LWL_DECLARE_ACCESSOR(litwindow::const_accessor, LWBASE_API)

#ifdef _NATIVE_WCHAR_T_DEFINED
LWL_DECLARE_ACCESSOR(wchar_t, LWBASE_API)
// else wchar_t maps to unsigned short which has already been specified
#endif
#endif

#endif

#pragma warning(pop)

#endif
