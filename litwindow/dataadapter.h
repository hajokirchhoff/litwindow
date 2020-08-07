/*
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library
* distribution, file LICENCE.TXT
* $Id: dataadapter.h,v 1.12 2007/10/26 11:52:59 Hajo Kirchhoff Exp $
*/
#ifndef _LITWINDOW_DATAADAPTER_
#define _LITWINDOW_DATAADAPTER_

#pragma warning(push)

#include "lwbase.hpp"
#include <string>
#include "dataadapterimp.h"

/** @file
This file contains the data adapter mechanism.
*/

namespace litwindow {

	/** @defgroup data_adapter Data adapters
	@{
	*/

	/** \defgroup data_adapter_reference Reference
	Data Adapter reference
	@{
	*/

	class const_container;
	class container;
	class const_aggregate;
	class aggregate;
    class converter_enum_info;
	using std::string;

	/** const_accessor objects are the most fundamental data adapters, providing basic, readonly access to objects.
	A const_accessor can point to objects of any kind and provides basic readonly access to the values of these objects.
	const_accessor treats the objects pointed to as 'atomic'. They can access the objects as a whole,
	return their values as string, test certain aspects of the object, but cannot access individual aspects - such as members - of the objects.
	Use aggregate if the object is an aggregate (struct or class) or container if the object is a container.
	*/
	class LWBASE_API const_accessor
	{
	protected:
		const_prop_ptr member_ptr;
		const void *this_ptr;
		const schema_entry *s_entry;
	public:
		///@name Constructors
		/// You usually do not use these constructors directly. Use litwindow::make_const_accessor to construct objects of this class.
		//@{
		const_accessor();
		const_accessor(const void *aThisPtr, const schema_base::const_iterator &aThisInfo);
		const_accessor(const void *aThisPtr, const schema_entry &e);
		//@}


		///@name Comparing accessors.
		//@{
		/** Compare two accessors with each other. */
		/** Two accessors are considered equal if they point to the same object. */
		bool operator == (const const_accessor &a) const;
		bool operator != (const const_accessor &a) const;
		/** The comparison operators allow maps and sets of accessors but have no other meaningful semantic. */
		bool operator < (const const_accessor &a) const;
		bool operator > (const const_accessor &a) const;
		/** is_alias_of is similar to operator== */
		bool is_alias_of(const const_accessor &c) const;
		//@}

		///\name Accessing the type of the accessor.
		///The type of an accessor is defined as the type of the object the accessor points to.
		//\{
		/** Test the type of accessors. */
		bool is_type(prop_t type) const;
		/** Test if the accessors have the same type. */
		bool is_type(const const_accessor &a) const;
		/** Test the type of the accessor. */
		template <typename Type>
		bool is_type() const;
		/// Get the type name of the object.
		const string type_name() const;
		const string get_type_name() const;
		/// return the type of the object.
		prop_t get_type() const;
		//\}

		///\name Testing capabilities of the accessor.
		///These functions test capabilities and attributes of the accessor.
		//\{
		/// Test if the accessor points to a const container.
		bool is_const_container() const;
		/// Test if the accessor points to a container.
		bool is_container() const;
		/// Test if the accessor points to an aggregate.
		bool is_aggregate() const;
		/// Test if the accessor points to a superclass.
		bool is_inherited() const;
		/// Test if the accessor points to a co-object (see co-object section for details).
		bool is_coobject() const;
		/// Test if the accessor points to an aggregate nested in another aggregate.
		bool is_nested() const;
		/// Test if the accessor points to an old style C vector
		bool is_c_vector() const;
        /// Test if the accessor is an enumeration
        bool is_enum() const;
        const converter_enum_info *get_enum_info() const;
		/** get the size of the original object.
		get_sizeof() returns 'sizeof(object)', the number of bytes occupied by
		the original object.
		@note get_sizeof() will return the size only if there is an actual memory location at which the object
		is located. It will return 0 if the object is accessed through get/set member functions - if the data adapter
		uses PROP_GetSet or one of the related macros to define the adapter.
		*/
		size_t get_sizeof() const;

		/// Test if the accessor points to an object that can be copied.
		bool has_copy() const
		{
			return get_type()->has_copy();
		}
		/** Test the validity of this accessor.
		Accessors - like pointers - can be invalid.
		Invalid accessors do not point to an object. Calling functions that access the value of an object
		for an invalid accessor is not allowed and will cause the program to crash.\n
		Calling functions to query the type is valid.
		const_accessor::is_valid tests the validity of an accessor.
		@return true if the accessor points to an object, false if the accessor is invalid.
		*/
		bool is_valid() const
		{
			return s_entry!=0;
		}
		/// Test if the accessor points to an integer object.
		bool is_int() const { return get_type()->is_int(); }
		//\}

		inline const void *get_this_ptr() const
		{
			return this_ptr;
		}
		inline const_prop_ptr get_member_ptr() const
		{
			return member_ptr;
		}

		///\name Misc.
		//@{
		/// Get the name of the object.
		/// If the object is a member of an aggregate, return the identifier of the member as it is defined in the source code. Returns "this" if it is a basic data type.
		inline string name() const
		{
			return s_entry->m_name;
		}
		inline string get_name() const
		{
			return name();
		}
		/// Get the name of the class of this object if it is a member of an aggregate. Returns 0 if it is a basic data type.
		inline string class_name() const
		{
			return s_entry->m_class_name;
		}

		const schema_entry &get_schema_entry() const
		{
			return *s_entry;
		}

		/** Return a string representation of the value of the object. */
		tstring to_string() const
		{
			return get_type()->to_string(s_entry, get_member_ptr());
		}
		//@}

		//        bool is_enum() const { return get_type()->is_enum(); }
		//        std::vector<tstring> get_enum_values() const { return get_type()->get_enum_values(); }

		typedef litwindow::const_container container_type;
		typedef litwindow::const_aggregate aggregate_type;

		/// Return a container accessor for this object.
		/// It is valid to call get_container for an object that isn't a container. If thats the case
		/// const_container::is_valid() will return false for the returned container accessor.
		const_container get_container() const;
		const_container get_const_container() const;

		/// Return an aggregate accessor for this object.
		/// It is valid to call get_aggregate for an object that isn't an aggregate. If thats the case,
		/// const_aggregate::is_valid() will return false for the returned aggregate accessor.
		const_aggregate get_aggregate() const;
		/// Return an 'int' representation of this object.
		int to_int() const { return get_type()->to_int(s_entry, get_member_ptr()); }
		/** Clone the underlying object.
		This function allocates a new object on the heap and copies the contents
		of the current object to the new object. @returns an accessor to the new object.
		@note Call const_accessor::destroy when you no longer need the clone.
		*/
		const_accessor clone() const;
		/** Destroy a cloned object.
		This function calls 'delete' for the object pointed to by this accessor.
		@note Calling 'destroy' for accessors that have not been created by @p clone
		or a similar function is illegal. The result is undefined. An access
		violation, memory corruption or other such things may happen.
		*/
		void destroy()
		{
			if (is_valid()) {
				get_type()->destroy(get_member_ptr());
				set_invalid();
			}
		}
		/** mark the accessor as 'invalid'.
		subsequent calls to is_valid() will return false.
		*/
		void set_invalid()
		{
			s_entry=0;
		}
		void clear() { set_invalid(); }

		//virtual size_t to_binary(void *destination, size_t destination_size) const = 0;
		//virtual size_t from_binary(void *source, size_t source_size) = 0;
		/** assert the validness of the accessor, throw an lwbase_error exception if it is invalid.
		*/
		void assert_valid() const
		{
			if (!is_valid())
				throw lwbase_error("accessor is invalid");
		}

		template <typename Value>
		Value get() const;

		/// store, possibly cast \p this into \p a
		void query_value(const accessor &a);
		template <typename Value>
		void query_value(Value &v);
		template <typename Value>
		void get(Value &v)
		{
			query_value(v);
		}
	};

	/** Objects of this class allow read-write access to a single property.
	An accessor can point to objects of any kind and provides basic readwrite access to the values of these objects.
	accessor treats the objects pointed to as 'atomic'. They can access the objects as a whole,
	return their values as string, test certain aspects of the object, but cannot access individual aspects - such as members - of the objects.
	Use aggregate if the object is an aggregate (struct or class) or container if the object is a container.
	*/
	class LWBASE_API accessor:public const_accessor
	{
	public:
		///@name Constructors
		/// You would not normally use these constructors, use make_accessor instead.
		//@{
		accessor() {}
		accessor(void *aThisPtr, const schema_base::const_iterator &aThisInfo)
			:const_accessor(aThisPtr, aThisInfo)
		{}
		accessor(void *aThisPtr, const schema_entry &e)
			:const_accessor(aThisPtr, e)
		{}
		accessor(void *aThisPtr, prop_t type)
			:const_accessor(aThisPtr, *type->get_schema_entry())
		{
		}
		//@}
		///@name get_functions
		//@{
		void *get_this_ptr() const { return const_cast<void*>(const_accessor::get_this_ptr()); }
		prop_ptr get_member_ptr() const { return const_cast<prop_ptr>(const_accessor::get_member_ptr()); }
		//@}
		/** assign the object a new value from a string representation
		*/
		size_t from_string(const tstring &value) const
		{
			return get_type()->from_string(s_entry, value, get_member_ptr());
		}

		/** assign an 'int' value to the object.
		*/
		void from_int(int value) const
		{
			get_type()->from_int(s_entry, value, get_member_ptr());
		}

		/** copy the object pointed to by @p a to @p this
		*/
		void from_accessor(const const_accessor &a) const;
		/** copy the object pointed to by @p a to @p this.
		same as from_accessor
		*/
		void assign(const const_accessor &a)
		{
			from_accessor(a);
		}

		typedef litwindow::container container_type;
		typedef litwindow::aggregate aggregate_type;

		/** get a container accessor for this object. */
		container get_container() const;
		/** get an aggregate accessor for this object. */
		aggregate get_aggregate() const;

		/** clone this object. Creates a new object on the heap,
		copy the value of this object to it and returns an accessor
		to the new object.
		@return an accessor to the cloned object
		@note Call accessor::destroy when you no longer need the clone.
		*/
		accessor clone() const
		{
			return get_type()->clone(s_entry, get_member_ptr());
		}

		void assign_value(const const_accessor &source);
		template <typename Value>
		void assign_value(const Value &v);
		template <>
		void assign_value(const int &i);

		template <typename Value>
		void set(const Value &v)
		{
			assign_value(v);
		}
	};

	inline const_accessor const_accessor::clone() const
	{
		return get_type()->clone(s_entry, get_member_ptr());
	}

	///\name Casting and type querying
	//@{
	//-----------------------------------------------------------------------------------------------------------//
	/** returns an accessor for a const_accessor. Removes the const'ness. Similar to const_cast<>. */
	inline accessor const_cast_accessor(const const_accessor &access)
	{
		return accessor(const_cast<prop_ptr>(access.get_this_ptr()), access.get_schema_entry());
	}

	/** tests the type of the object pointed to by accessor @p p.
	@returns true if the object is of type @p Value.
	is_type<int>(p) will return true if p is an accessor for an int object. Likewise
	is_type<string>(p) returns true if p is an accessor for a string object.

	Example:
	@code
	litwindow::accessor p=make_accessor(g_variable);
	if (litwindow::is_type<foobar>(p)) {
	cout << accessor_as_debug(p) << " is of type foobar." << endl;
	}
	@endcode
	*/
	template <class Value>
	bool is_type(const const_accessor &p)
	{
		return p.is_valid() && p.is_type(get_prop_type<Value>());
	}

	/** typed_const_accessor is a const_accessor with additional get/set functions that
	work with the actual type of the object.
	typed_const_accessor allows you to access the object if you already know its type. make_const_accessor
	hides the type of the object inside an accessor. To access data you have to go through the to_/from_ accessor member
	functions. But there are scenarios where you as a library programmer expect an accessor to have a specific type, or
	one out of a set of types. typed_const_accessor exposes the type of the object again, effectively reversing the
	make_const_accessor operation.

	Use dynamic_cast_accessor to convert a const_accessor to a typed_const_accessor<Type>. @b Example:
	\code
	const_accessor a;             // a const accessor pointing to an object of unknown type
	typed_const_accessor<int> an_int=dynamic_cast_accessor<int>(a);   // cast it to an <int> accessor
	if (an_int.is_valid()) {      // yes, a did point to an integer
	int i=an_int.get();        // access the int object
	} else {
	printf("The const_accessor 'a' does not point to an 'int' object.\n");
	printf("It points to an object of type '%s' instead.", a.get_type_name().c_str());
	}
	\endcode
	\note There are some subtle pitfalls to avoid. While C++ pointers always point to an object
	in memory, which can be manipulated directly, accessors are a mixture between pointers and member pointers.
	As long as the accessor points to an actual object at a memory location, all typed_const_accessor methods will
	work as expected. But if the data adapter defines a get/set method pair to access the object, calling
	typed_const_accessor::get_ptr() for example will not work, since the object is actually the return value
	of a function call. There is no memory location that could be returned and get_ptr() will return 0.
	When using typed_const_accessor, always keep in mind that the underlying object may not be a member
	variable, but actually a pair of get/set methods.
	*/
	template <class Value>
	class typed_const_accessor:public const_accessor
	{
	protected:
		converter_value_base<Value> *type_ptr;
	public:
		typed_const_accessor(const const_accessor &p)
			:const_accessor(p)
		{
			//if (p.is_type(get_prop_type<Value>()))
			type_ptr=dynamic_cast<converter_value_base<Value> *>(p.get_type());
			//else
			//    type_ptr=0;
		}
		typed_const_accessor()
			:type_ptr(0)
		{}
		/** return an untyped const_accessor from this typed_const_accessor. This is the reverse operation of dynamic_cast_accessor<> */
		const_accessor get_const_accessor() const
		{
			return *this;
		}


		/** Return the value of the object pointed to by this accessor.
		This function accesses the object directly rather than using string representations.
		@code
		typed_const_accessor<float> p=dynamic_cast_accessor<float>(someAccessor);
		if (p.is_valid()) {
		float aFloat;
		p.get(aFloat);
		} else {
		printf("error: someAccessor is not of type float.");
		}
		@endcode
		will copy the float pointed to by @p someAccessor to @p aFloat
		@returns the actual value & type of the object.
		*/
		void get(Value &v) const
		{
			type_ptr->get_value(v, &get_schema_entry(), get_member_ptr());
		}
		/** return the value of the object pointed to by this accessor.
		Same as get(Value &v) except that it returns the value.
		*/
		const Value get() const
		{
			Value v;
			get(v);
			return v;
		}
		/** get a typed pointer to the original object.
		get_ptr() returns a pointer to the object pointed to by this accessor.
		@note This method works only if the accessor is actually pointing to
		an object or member variable of an object. It will return a NULL pointer
		in some cases, such as when the accessor uses get/set methods to access the value.
		Use of typed_const_accessor::get is generally preferred. @b Example:
		\code
		LWL_BEGIN_ADAPTER(MyStruct)
		PROP(m_member)
		PROP_GetSet(Value)      // calls MyStruct::GetValue to get the value
		LWL_END_ADAPTER()

		MyStruct s;
		// get a const_accessor pointing to the Value member of MyStruct
		const_accessor an_accessor=make_const_aggregate(s)["Value"];
		// cast it to a typed_const_accessor<int>
		typed_const_accessor<int> a=dynamic_cast_accessor(an_accessor);
		// will call MyStruct::GetValue and copy the result to 'v'
		int v=a.get();
		// will return NULL, since there is no actual memory location for 'Value'
		int *ptr=a.get_ptr();
		\endcode
		Calling a.get() will work fine. The accessor will actually call MyStruct::GetValue() and store the return value
		in @p v. But a.get_ptr() will return NULL, since there is no actual memory location for the member @p Value.
		*/
		const Value *get_ptr() const
		{
			return type_ptr->get_ptr(&get_schema_entry(), get_member_ptr());
		}
		size_t get_sizeof() const
		{
			return type_ptr->get_sizeof(&get_schema_entry());
		}
		/** get the number of elements in a c-style vector.
		If the underlying object is a C vector, return the number of elements in the vector. Return '1' if it isn't a vector
		or is a vector with only 1 element and '0' if the object cannot be accessed directly but only through get/set
		member functions (see get_ptr() for details).
		*/
		size_t get_count() const
		{
			return get_sizeof()/sizeof(Value);
		}
		/** Tests if the object is actually a C vector. */
		bool is_vector() const
		{
			return get_count()>1;
		}
		/** get a typed reference to the original object.
		get_ref returns a reference to the object pointed to by this accessor.
		@note Not all accessors support this, in which case get_ref will
		throw a runtime_exception. Use of typed_const_accessor::get is generally preferred. See get_ptr() for details.
		*/
		const Value &get_ref() const
		{
			if (get_ptr()==0)
				throw lwbase_error("accessor does not support get_ref");
			return *get_ptr();
		}
		/** test validity of the accessor.
		@returns true if the accessor is valid and points to an object.
		*/
		bool is_valid() const
		{
			return type_ptr!=0;
		}
		/** assert the validness of the accessor, throw an exception if it is invalid.
		*/
		void assert_valid() const
		{
			const_accessor::assert_valid();
			if (!is_valid())
				throw lwbase_error("accessor '"+accessor_as_debug(*this)+" is not of type "+get_prop_type<Value>()->get_type_name());
		}
	};

	/** typed_accessor is an accessor with additional type information and get/set functions.
	Used with dynamic_cast_accessor.
	*/
	template <class Value>
	class typed_accessor:public typed_const_accessor<Value>
	{
		typedef typed_const_accessor<Value> Inherited;
	public:
		typed_accessor(const accessor &a)
			:typed_const_accessor<Value>(a)
		{}
		/** get an accessor from the typed_accessor. This is the reverse operation of dynamic_cast_accessor.
		*/
		accessor get_accessor() const
		{
			return const_cast_accessor(Inherited::get_const_accessor());
		}
		/** assign a new value to the object pointed to by this accessor.
		*/
		void set(const Value &v)
		{
			Inherited::type_ptr->set_value(v, &Inherited::get_schema_entry(), const_cast<prop_ptr>(Inherited::get_member_ptr()));
		}
		Value *get_ptr() const
		{
			return const_cast<Value*>(Inherited::get_ptr());
		}
		Value &get_ref() const
		{
			return *get_ptr();
		}
	};

	/** Cast an accessor to a typed_accessor.
	*/
	template <class Value>
	inline typed_accessor<Value> dynamic_cast_accessor(const accessor &p)
	{
		return typed_accessor<Value>(p);
	}

	/** Cast a const_accessor to a typed_const_accessor
	*/
	template <class Value>
	inline typed_const_accessor<Value> dynamic_cast_accessor(const const_accessor &p)
	{
		return typed_const_accessor<Value>(p);
	}
	//@}
	
	template <typename Value>
	void const_accessor::query_value(Value &v)
	{
		typed_const_accessor<Value> a(dynamic_cast_accessor<Value>(*this));
		if (a.is_valid())
			a.get(v);
		else
			query_value(make_accessor(v));
	}

	template <typename Value>
	Value const_accessor::get() const
	{
		return dynamic_cast_accessor<Value>(*this).get();
	}

	//-----------------------------------------------------------------------------------------------------------//
	//-----------------------------------------------------------------------------------------------------------//
	/** const_aggregate provides readonly access to aggregate objects.
	A const_aggregate adapter allows access to the member variables of a @c struct or @c class. It implements
	a random access iterator that iterates over all members that appear in the data adapter definition.
	*/
	class const_aggregate
	{
	protected:
		const void *this_ptr;
		const schema_base *thisSchema;
	public:
		///\name Constructors
		/// You don't usually construct objects directly. Use litwindow::make_const_aggregate instead.
		//@{
		const_aggregate(const void *aThisPtr, const schema_base &aThisSchema):this_ptr(aThisPtr), thisSchema(&aThisSchema) {}
		const_aggregate()
			:this_ptr(0), thisSchema(0) {}
		//@}
		/**Test if the accessor points to an object and the value functions can be used.
		\copydoc litwindow::const_accessor::is_valid() */
		bool is_valid() const
		{
			return this_ptr!=0;
		}
		const_accessor self() const
		{
			return const_accessor(get_this_ptr(), get_schema_entry());
		}
		const void *get_this_ptr() const
		{
			return this_ptr;
		}
		const schema_entry &get_schema_entry() const
		{
			return thisSchema->get_this_schema_entry();
		}
		/** Iterate over the members of the aggregate. */
		class const_iterator
		{
			const void *this_ptr;
			const_accessor a;
			schema_base::const_iterator it;
		public:
			const_iterator() {}
			const_iterator(const void *aThisPtr, const schema_base::const_iterator &initial)
				:it(initial)
				,this_ptr(aThisPtr)
			{
			}
			/// Access the member. Return a const_accessor that points to the current member. */
			const_accessor operator *()
			{
				return const_accessor(this_ptr, it);
			}
			/// Access the member. Return a pointer to a const_accessor pointing to the current member. */
			const const_accessor *operator->()
			{
				a=const_accessor(this_ptr, it);
				return &a;
			}

			/// Compare two iterators. Iterators are equal if they point to the same member of the object. */
			bool operator==(const const_iterator &i) const
			{
				return it==i.it;
			}

			bool operator!=(const const_iterator &i) const
			{
				return !operator==(i);
			}

			/// Move the iterator to the next member of the object and return the new iterator. ++ Prefix operator.
			const_iterator &operator++()
			{
				++it;
				return *this;
			}

			/// Return the current iterator and move the position to the next member. ++ Postfix operator.
			const_iterator operator++(int)
			{
				const_iterator rc(*this);
				++it;
				return rc;
			}

			/// Move the iterator to the previous member of the object.
			const_iterator &operator--()
			{
				--it;
				return *this;
			}
			/// Move the iterator to the previous member of the object.
			const_iterator operator--(int)
			{
				const_iterator rc(*this);
				--it;
				return rc;
			}
		};

		/**@name Finding members
		These functions search aggregate members by name. */
		//@{
		/// find the name using C++ scope rules.
		/** This method searches the data adapter for a member named @p propName. It follows C++ scope rules.
		If it finds a matching member in the current scope it will return an iterator to this member. If not it will
		search the inheritance hierarchy upwards until it finds a matching member, much like the C++ compiler does.
		It will return 'end()', if it does not find a member of this name.

		The name passed into @p propName can be a valid C++ identifier with an optional scope. Valid examples are
		- m_my_name
		- Inherited::m_member
		- inherited.m_member
		- m_aggregate.m_member
		*/
		std::pair<const_iterator,bool> LWBASE_API find_scope(const string &propName) const
		{
			return find_scope(propName, false);
		}

		/// find the name using C++ scope rules but also search member aggregates.
		/** This method works like find_scope, but it searches not only the inheritance hierarchy, but member aggregates as well.
		It does not have a C++ equivalent.
		*/
		std::pair<const_iterator,bool> LWBASE_API find_anywhere(const string &propName) const
		{
			return find_scope(propName, true);
		}

		std::pair<const_iterator,bool> LWBASE_API find_scope(const string &propName, bool search_member_aggregates) const;

		/// find a member of the name @p propName.
		/** Unlike find_scope or find_anywhere this method searches only in the immediate data adapter definition. It does
		not find members from inherited classes or aggregates. */
		const_iterator find(const char *propName) const
		{
			return const_iterator(get_this_ptr(), thisSchema->find(propName));
		}
		const_iterator find(const string &propName) const
		{
			return find(propName.c_str());
		}
		/** Return an accessor to the member @p propName or throw an exception if no such member exists. This method
		allows convenient access to members that are required to exist. It is the runtime equivalent of the . operator.
		\code
		// assign the value 10 to a member, C++ syntax
		myobject.mymember = 10;
		// assign the value 10 to a member, data adapter syntax
		myobject["mymember"].from_string("10")
		\endcode
		*/
		const_accessor operator[](const char *propName) const
		{
			std::pair<const_iterator,bool> it=find_scope(propName);
			if (it.second==false)
				throw lwbase_error(propName+string(": no such property"));
			return *it.first;
		}
		/// Same as operator[](const char*)
		const_accessor operator[](const string &propName) const
		{
			return operator[](propName.c_str());
		}
		//@}
		/// Get an iterator pointing to the first member of the aggregate
		const_iterator begin() const { return const_iterator(get_this_ptr(), thisSchema->begin()); }
		/// Get an iterator pointing behind the last member of the aggregate
		const_iterator end() const { return const_iterator(get_this_ptr(), thisSchema->end()); }
		/// Convert the const_aggregate data adapter into a simple const_accessor data adapter
		const_accessor get_accessor() const
		{
			return self();
		}

		/// Get the C++ identifier of the class as it is used in the source code.
		string class_name() const { return thisSchema->class_name(); }
		string get_class_name() const { return class_name(); }
		const schema_base &get_schema() const { return *thisSchema; }

		/*! return true if @p c is an alias of this */
		bool is_alias_of(const const_aggregate &c) const
		{
			return get_this_ptr()==c.get_this_ptr();
		}

		/// Compare two data adapters. They are considered equal if they point to the same object.
		bool operator==(const const_aggregate &c) const
		{
			return c.is_alias_of(c);
		}
		bool operator!=(const const_aggregate &c) const
		{
			return !operator==(c);
		}
		/// Define an ordering relation so that the aggregate can be used with map or set. Has no other meaningful semantic.
		bool operator<(const const_aggregate &c) const
		{
			return get_this_ptr() < c.get_this_ptr();
		}

		typedef const_iterator _iter;
		typedef const_iterator iterator_type;
	};

	/** Base implementation for an aggregate (non-const) of properties.
	A const_aggregate adapter allows access to the member variables of a @c struct or @c class. It implements
	a random access iterator that iterates over all members that appear in the data adapter definition.
	*/
	class aggregate:public const_aggregate
	{
	public:
		aggregate(void *aThisPtr, const schema_base &aThisSchema):const_aggregate(aThisPtr, aThisSchema) {}
		aggregate() {}
		accessor self() const
		{
			return accessor(get_this_ptr(), get_schema_entry());
		}
		void *get_this_ptr() const
		{
			return const_cast<void*>(const_aggregate::get_this_ptr());
		}

		/** Iterate over the members of the aggregate. See const_aggregate::const_iterator for details. */
		class iterator
		{
			void *this_ptr;
			accessor a;
			schema_base::const_iterator it;
		public:
			iterator() {}
			iterator(void *aThisPtr, const schema_base::const_iterator &initial)
				:it(initial)
				,this_ptr(aThisPtr)
			{
			}
			accessor operator *()
			{
				return accessor(this_ptr, it);
			}
			accessor *operator->()
			{
				a=accessor(this_ptr, it);
				return &a;
			}
			bool operator==(const iterator &i) const
			{
				return it==i.it;
			}

			bool operator!=(const iterator &i) const
			{
				return !operator==(i);
			}

			iterator &operator++()
			{
				++it;
				return *this;
			}

			iterator operator++(int)
			{
				iterator rc(*this);
				++it;
				return rc;
			}
			iterator &operator--()
			{
				--it;
				return *this;
			}
			iterator operator--(int)
			{
				iterator rc(*this);
				--it;
				return rc;
			}
		};

		std::pair<iterator,bool> LWBASE_API find_scope(const string &propName) const
		{
			return find_scope(propName, false);
		}

		std::pair<iterator, bool> LWBASE_API find_anywhere(const string &propName) const
		{
			return find_scope(propName, true);
		}

		std::pair<iterator, bool> LWBASE_API find_scope(const string &propName, bool search_member_aggregates) const;

		iterator find(const char *propName) const
		{
			return iterator(get_this_ptr(), thisSchema->find(propName));
		}
		iterator find(const string &propName) const
		{
			return find(propName.c_str());
		}
		accessor operator[](const char *propName) const
		{
			std::pair<iterator, bool> it=find_scope(propName);
			if (it.second==false)
				throw lwbase_error(propName+string(": no such property"));
			return *it.first;
		}
		accessor operator[](const string &propName) const
		{
			return operator[](propName.c_str());
		}
		iterator begin() const { return iterator(get_this_ptr(), thisSchema->begin()); }
		iterator end() const { return iterator(get_this_ptr(), thisSchema->end()); }
		accessor get_accessor() const
		{
			return self();
		}

		typedef iterator _iter;
		typedef iterator iterator_type;
	};

	/// execute f() for each member of an aggregate including inherited members
	template <class _A, class _F>
	_F for_each_member(_A &a, _F &f)
	{
		typename _A::iterator_type i;
		for (i=a.begin(); i!=a.end(); ++i) {
			if (i->is_aggregate())
				for_each_member(i->get_aggregate(), f);
			else
				f(*i);
		}
		return f;
	}



	/** Create a const_accessor for an object.
	@returns a const_accessor pointing to the object passed as parameter @p v.
	const_accessor object can read, but not write the object they point to.
	@note This creates a @em pointer to the object. The object itself is not
	modified in any way. Its lifetime must be longer than the lifetime of the
	accessor.
	*/
	template <class Value>
	inline const_accessor make_const_accessor(const Value &v)
	{
		return const_accessor(&v, get_schema_entry(v));
	}

	/** Create an accessor for an object.
	@returns an accessor pointing to the object passed as parameter @p v.
	accessor objects can read and write objects they point to.
	@note This creates a @em pointer to the object. The object itself is not
	modified in any way. Its lifetime must be longer than the lifetime of the
	accessor.
	*/
	template <class Value>
	inline accessor make_accessor(Value &v)
	{
		return accessor(&v, get_schema_entry(v));
	}

    inline accessor reinterpret_accessor(prop_t target_type, const accessor &source)
    {
        return accessor(source.get_this_ptr(), target_type);
    }

	template <class Value>
	inline const_aggregate make_const_aggregate(Value &v)
	{
		return const_aggregate(&v, get_schema(v));
	}

	template <class Value>
	inline aggregate make_aggregate(Value &v)
	{
		return aggregate(&v, get_schema(v));
	}

	template <>
	inline void accessor::assign_value(const int &i)
	{
		if (is_int())
			from_int(i);
		else
			assign_value(make_const_accessor(i));
	}


	template <typename Value>
	void accessor::assign_value(const Value &v)
	{
		typed_accessor<Value> t(dynamic_cast_accessor<Value>(*this));
		if (t.is_valid())
			t.set(v);
		else {
			assign_value(make_const_accessor(v));
		}
	}

	inline const_aggregate const_accessor::get_aggregate() const
	{
		const schema_base *theSchema=get_type()->get_schema();
		if (theSchema==0)
			return const_aggregate();
		return const_aggregate(get_member_ptr(), *get_type()->get_schema());
	}

	inline aggregate accessor::get_aggregate() const
	{
		const schema_base *theSchema=get_type()->get_schema();
		if (theSchema==0)
			return aggregate();
		return aggregate(get_member_ptr(), *get_type()->get_schema());
	}

	inline accessor create_object(prop_t object_type)
	{
		return accessor(object_type->create(), *object_type->get_schema_entry());
	}

	inline accessor create_object_by_name(const char *by_name)
	{
		return create_object(get_prop_type_by_name(by_name));
	}

	inline void destroy_object(accessor &a)
	{
		if (a.is_valid())
			a.get_type()->destroy(a.get_this_ptr());
		a.set_invalid();
	}

	//! \if developers
	template <class Value>
	inline accessor converter_value_base<Value>::clone(const schema_entry *e, const_prop_ptr member_ptr) const
	{
		Value *v=new Value;
		std::unique_ptr<Value> guard(v);
		get_value(*v, e, member_ptr);
		accessor rc=make_accessor(*v);
		guard.release();
		return rc;
	}

	template <class Value>
	inline accessor converter_abstract_base<Value>::clone(const schema_entry *, const_prop_ptr) const
	{
		throw not_implemented_error("clone (abstract class!)");
	}

	//! \endif

	// end of data_adapter_reference
	//!@}

	// end of data_adapter
	//!@}

};

#ifndef DOXYGEN_INVOKED
#include "./dataadaptercontainerimp.h"
#endif

namespace litwindow {
	//-----------------------------------------------------------------------------------------------------------//
	class container:public accessor
	{
	public:
		container()
		{
		}
		container(const accessor &a)
			:accessor(a)
		{}
		class iterator
		{
			accessor a;
			container_iterator_imp_base *it;
		public:
			iterator(container_iterator_imp_base *_it=0)
				:it(_it)
			{}
			~iterator()
			{
				delete it;
			}
			iterator(const iterator &i)
				:it(i.it ? i.it->clone() : 0)
			{
			}
			const iterator &operator=(const iterator &i)
			{
				delete it;
				it=i.it ? i.it->clone() : 0;
				return *this;
			}
			bool operator==(const iterator &i) const
			{
				return (it && i.it && *it==*i.it);
			}
			bool operator!=(const iterator &i) const
			{
				return !operator==(i);
			}
			iterator &operator ++()
			{
				it->inc();
				return *this;
			}
			iterator &operator+=(size_t off)
			{
				it->advance(off);
				return *this;
			}
			accessor operator *()
			{
				return it->get();
			}
			accessor *operator->()
			{
				a=it->get();
				return &a;
			}
			iterator operator + (size_t off) const
			{
				iterator rc(*this);
				return rc+=off;
			}
			bool insert_into(container &where, const accessor &what)
			{
				return it->insert_into(where, what);
			}
			bool erase_from(container &where)
			{
				return it->erase_from(where);
			}
			prop_t get_element_type() const
			{
				return it->get_element_type();
			}
		};

		iterator begin() const
		{
			return iterator(get_type()->get_begin(s_entry, get_member_ptr()));
		}
		iterator end() const
		{
			return iterator(get_type()->get_end(s_entry, get_member_ptr()));
		}
		iterator at(size_t idx) const
		{
			return begin()+idx;
		}
		bool insert(iterator &i, const accessor &p)
		{
			return i.insert_into(*this, p);
		}
		bool erase(iterator &i)
		{
			return i.erase_from(*this);
		}

		typedef iterator _iter;
		typedef iterator iterator_type;

		/// return the type of the elements in the container
		prop_t get_element_type() const
		{
			return end().get_element_type();
		}
	};



	//-----------------------------------------------------------------------------------------------------------//
	class const_container:public const_accessor
	{
	public:
		const_container(const const_accessor &a)
			:const_accessor(a)
		{}
		class iterator
		{
			const_accessor a;
			const_container_iterator_imp_base *it;
		public:
			iterator(const_container_iterator_imp_base *_it=0)
				:it(_it)
			{}
			~iterator()
			{
				delete it;
			}
			iterator(const iterator &i)
				:it(i.it ? i.it->clone() : 0)
			{
			}
			const iterator &operator=(const iterator &i)
			{
				delete it;
				it=i.it ? i.it->clone() : 0;
				return *this;
			}
			bool operator==(const iterator &i) const
			{
				return (it && i.it && *it==*i.it);
			}
			bool operator!=(const iterator &i) const
			{
				return !operator==(i);
			}
			iterator &operator ++()
			{
				it->inc();
				return *this;
			}
			const_accessor operator *()
			{
				return it->get();
			}
			const_accessor *operator ->()
			{
				a=it->get();
				return &a;
			}
			prop_t get_element_type() const
			{
				return it->get_element_type();
			}
		};

		typedef iterator const_iterator;    ///< they are the same here. const_container::iterator is a const_iterator.

		typedef const_iterator _iter;
		typedef const_iterator iterator_type;

		iterator begin() const
		{
			return iterator(get_type()->get_const_begin(s_entry, get_member_ptr()));
		}
		iterator end() const
		{
			return iterator(get_type()->get_const_end(s_entry, get_member_ptr()));
		}

		/// return the type of the elements in the container
		prop_t get_element_type() const
		{
			return end().get_element_type();
		}
	};

	template <class Value>
	inline const_container make_const_container(Value &v)
	{
		return const_container(make_const_accessor(v));
	}
	template <class Value>
	inline container make_container(Value &v)
	{
		return container(make_accessor(v));
	}
	inline const_container const_accessor::get_container() const
	{
		if (!is_const_container())
			throw lwbase_error("is not a container");
		return const_container(*this);
	}
	inline const_container const_accessor::get_const_container() const
	{
		 return get_container();
	}

	inline container accessor::get_container() const
	{
		if (!is_container())
			throw litwindow::lwbase_error("is not a container");
		return container(*this);
	}
};


namespace litwindow {
	tstring LWBASE_API accessor_as_debug(const const_accessor &a);
	tstring LWBASE_API accessor_as_debug(const const_accessor &a, bool show_type);

	template <class Value>
	inline tstring as_debug(const Value &v)
	{
		return accessor_as_debug(make_const_accessor(v));
	}

	template <>
	inline tstring as_debug<const_accessor>(const const_accessor &v)
	{
		return accessor_as_debug(v);
	}

	template <>
	inline tstring as_debug<accessor>(const accessor &v)
	{
		return accessor_as_debug(v);
	}

	// export commonly used templates
#pragma warning(disable:4231 4275 4251)
	STL_EXPORT_VECTOR(const accessor*);
	STL_EXPORT_VECTOR(accessor);
	STL_EXPORT_VECTOR(const const_accessor*);
	STL_EXPORT_VECTOR(litwindow::const_accessor);
	STL_EXPORT_VECTOR(aggregate);
	STL_EXPORT_VECTOR(const_aggregate);

};


// Implementation starts here
namespace litwindow {

	inline void accessor::from_accessor(const const_accessor &a) const
	{
		get_type()->from_accessor(s_entry, a, get_member_ptr());
	}

	inline const_accessor::const_accessor()
		:member_ptr(0)
		,this_ptr(0)
		,s_entry(0)
	{
	}
	inline const_accessor::const_accessor(const void *aThisPtr, const schema_base::const_iterator &aThisInfo)
		:this_ptr(aThisPtr)
		,member_ptr(aThisInfo->member_ptr(aThisPtr))
		,s_entry(& (*aThisInfo))
	{}
	inline const_accessor::const_accessor(const void *aThisPtr, const schema_entry &e)
		:this_ptr(aThisPtr)
		,member_ptr(e.member_ptr(aThisPtr))
		,s_entry(&e)
	{}

	/** Test two accessors for equality. Accessors are equal if they point to the same object.
	@returns true if the accessors access the same object (similar to pointer equality). */
	inline bool const_accessor::operator == (const const_accessor &a) const
	{
		return get_member_ptr()==a.get_member_ptr() && get_schema_entry().compare(a.get_schema_entry())==0;
	}
	/** @returns true if the accessors point to @b different objects. */
	inline bool const_accessor::operator != (const const_accessor &a) const
	{
		return !operator==(a);
	}
	/** @returns true if @p this is smaller than @p a. Used for map<const_accessor, ...>. */
	inline bool const_accessor::operator < (const const_accessor &a) const
	{
		return get_member_ptr()<a.get_member_ptr() || (get_member_ptr()==a.get_member_ptr() && get_schema_entry().compare(a.get_schema_entry()) < 0);
	}
	/** @returns true if @p this is larger than @p a. Used for map<const_accessor, ...>. */
	inline bool const_accessor::operator > (const const_accessor &a) const
	{
		return a < *this;
	}
	/** \return true if @p c is an alias of this. Similar to operator== */
	inline bool const_accessor::is_alias_of(const const_accessor &c) const
	{
		return operator==(c);
	}
	/** is_type tests if the given accessor is of a specific type.
	@return true if @p this object is of @p type.
	@note Using the template function is_type<Type>(a) is recommended instead. */
	inline bool const_accessor::is_type(prop_t type) const
	{
		return is_valid() && is_type_alias(type, get_type());
	}
	/** Test if the accessors @p this and \p a point to objects of the same type.
	\return true if the objects pointed to by the accessors are of the same type. */
	inline bool const_accessor::is_type(const const_accessor &a) const
	{
		return is_type(a.get_type());
	}
	/** Test if the accessor is of type TEMPLATE parameter. */
	template <typename Type>
	inline bool const_accessor::is_type() const
	{
		return is_type(get_prop_type<Type>());
	}
	/** Query the name of the @b type of the object pointed to by this accessor. The name of a type
	is specified with the DECLARE_ADAPTER_TYPE/IMPLEMENT_ADAPTER_TYPE macros.
	\return name of the type
	*/
	inline const string const_accessor::type_name() const
	{
		return get_type()->get_type_name();
	}
	/** Same as type_name(). */
	inline const string const_accessor::get_type_name() const
	{
		return type_name();
	}
	inline prop_t const_accessor::get_type() const
	{
		/** This function queries the type of the object pointed to by this accessor. Use this function
		to compare types of accessors or to create new objects of the same type using create_object.
		\return a type object representing the type of this accessor.
		*/
		return s_entry->m_type;
	}
	inline bool const_accessor::is_container() const
	{
		/** Tests if the object pointed to is a container. Use this function to test
		if get_container() would return a valid container adapter. */
		return get_type()->is_container();
	}
	inline bool const_accessor::is_const_container() const
	{
		/** Tests if the object pointed to is a container. Use this function to test
		if get_const_container() would return a valid container adapter. */
		return get_type()->is_const_container();
	}

	/** Tests if the object pointed to is an aggregate. Use this function to test
	if get_aggregate() would return a valid aggregate adapter. */
	inline bool const_accessor::is_aggregate() const
	{
		return get_type()->has_schema();
	}
	inline bool const_accessor::is_inherited() const
	{
		return get_schema_entry().is_inherited();
	}
	inline bool const_accessor::is_coobject() const
	{
		return get_schema_entry().is_coobject();
	}
	inline bool const_accessor::is_nested() const
	{
		return get_schema_entry().is_nested();
	}
	inline bool const_accessor::is_c_vector() const
	{
		return get_type()->is_c_vector();
	}
    inline bool const_accessor::is_enum() const
    {
        return get_type()->is_enum();
    }
    inline const converter_enum_info *const_accessor::get_enum_info() const
    {
        return get_type()->get_enum_info();
    }

	inline size_t const_accessor::get_sizeof() const
	{
		return get_type()->get_sizeof(s_entry);
	}

#ifndef DOXYGEN_INVOKED
#ifdef NOT
	/// Return true if the type is an integer type.
	bool is_int() const
	{
		return get_type()->is_int();
	}

	inline const void *get_this_ptr() const
	{
		return this_ptr;
	}
	inline const_prop_ptr get_member_ptr() const
	{
		return member_ptr;
	}

	//@{
	/// Get the name of this object if it is a member of an aggregate. Returns "this" if it is a basic data type.
	inline string name() const
	{
		return s_entry->m_name;
	}
	inline string get_name() const
	{
		return name();
	}
	//@}
	/// Get the name of the class of this object if it is a member of an aggregate. Returns 0 if it is a basic data type.
	inline string class_name() const
	{
		return s_entry->m_class_name;
	}

	/** @name Accessing the type
	These functions access or test the type of the object pointed to by the accessor.
	*/
	//@{
	//@}

	const schema_entry &get_schema_entry() const
	{
		return *s_entry;
	}

	/** Return a string representation of the value of the object. */
	tstring to_string() const
	{
		return get_type()->to_string(s_entry, get_member_ptr());
	}
	/// Return an 'int' representation of this object.
	int to_int() const { return get_type()->to_int(s_entry, get_member_ptr()); }

	//        bool is_enum() const { return get_type()->is_enum(); }
	//        std::vector<tstring> get_enum_values() const { return get_type()->get_enum_values(); }

	typedef litwindow::const_container container_type;
	typedef litwindow::const_aggregate aggregate_type;

	/// Return a container accessor for this object.
	/// It is valid to call get_container for an object that isn't a container. If thats the case
	/// const_container::is_valid() will return false for the returned container accessor.
	const_container get_container() const;

	/// Return an aggregate accessor for this object.
	/// It is valid to call get_aggregate for an object that isn't an aggregate. If thats the case,
	/// const_aggregate::is_valid() will return false for the returned aggregate accessor.
	const_aggregate get_aggregate() const;

	/// Returns true if the underlying object can be copied.
	bool has_copy() const
	{
		return get_type()->has_copy();
	}


	/** Test the validity of this accessor.
	Accessors - like pointers - can be invalid.
	Invalid accessors do not point to an object. Calling the value functions
	on an invalid accessor is not allowed and will cause the program to crash.
	const_accessor::is_valid tests the validity of an accessor.
	@return true if the accessor points to an object, false if the accessor is invalid.
	*/
	bool is_valid() const
	{
		return s_entry!=0;
	}
	/** Clone the underlying object.
	This function allocates a new object on the heap and copies the contents
	of the current object to the new object. @returns an accessor to the new object.
	@note Call const_accessor::destroy when you no longer need the clone.
	*/
	const_accessor clone() const;
	/** Destroy a cloned object.
	This function calls 'delete' for the object pointed to by this accessor.
	@note Calling 'destroy' for accessors that have not been created by @p clone
	or a similar function is illegal. The result is undefined. An access
	violation, memory corruption or other such things may happen.
	*/
	void destroy()
	{
		if (is_valid()) {
			get_type()->destroy(get_member_ptr());
			set_invalid();
		}
	}
	/** mark the accessor as 'invalid'.
	subsequent calls to is_valid() will return false.
	*/
	void set_invalid()
	{
		s_entry=0;
	}

	//virtual size_t to_binary(void *destination, size_t destination_size) const = 0;
	//virtual size_t from_binary(void *source, size_t source_size) = 0;
	/** assert the validness of the accessor, throw an exception if it is invalid.
	*/
	void assert_valid() const
	{
		if (!is_valid())
			throw lwbase_error("accessor is invalid");
	}
#endif
#endif

	template <class Value>
	void converter_value_base<Value>::from_accessor(const schema_entry *e, const const_accessor &a, prop_ptr member_ptr)
	{
		typed_const_accessor<Value> v=dynamic_cast_accessor<Value>(a);
		if (!v.is_valid())
			throw litwindow::lwbase_error("type mismatch");
		set_value(v.get(), e, member_ptr);
	}
	/* special cases for <accessor> and <const_accessor> */
	template <>
	inline void converter_value_base<accessor>::from_accessor(const schema_entry *e, const const_accessor &a, prop_ptr member_ptr)
	{
		set_value(const_cast_accessor(a), e, member_ptr);
	}
	template <>
	inline void converter_value_base<const_accessor>::from_accessor(const schema_entry *e, const const_accessor &a, prop_ptr member_ptr)
	{
		set_value(a, e, member_ptr);
	}

	/** converter class calling member functions to get or set a value. */
	template <class Value, class PropClass, class GetPointer, class SetPointer>
	class converter_with_getset:public converter_value_base<Value>
	{
		inline const PropClass *this_ptr(const_prop_ptr aPtr) const
		{
			return static_cast<const PropClass*>(aPtr);
		}
		inline PropClass *this_ptr(prop_ptr aPtr)
		{
			return static_cast<PropClass*>(aPtr);
		}
		prop_t  m_actual_type;
	public:
		converter_with_getset(const string &type_name="")
			:converter_value_base<Value>(type_name, 0), m_actual_type(get_prop_type<Value>())
		{
		}
		//virtual bool has_schema() const
		//{
		//	return m_actual_type->has_schema();
		//}
		//virtual const schema_base *get_schema() const
		//{ 
		//	return m_actual_type->get_schema(); 
		//}
		//virtual const schema_entry* get_schema_entry()
		//{
		//	return m_actual_type->get_schema_entry();
		//}
		virtual bool is_int() const { return m_actual_type->is_int(); }
		string get_type_name() const { return m_actual_type->get_type_name(); }
		tstring to_string(const schema_entry *entry, const_prop_ptr a_this_ptr)
		{
			GetPointer getter=GetterSetterPointer<GetPointer, SetPointer>::get_getter(entry->m_getter_setter);
			Value v=(this_ptr(a_this_ptr)->*getter)();
			const_accessor a(make_const_accessor(v));
			return a.to_string();
		}
		size_t from_string(const schema_entry *entry, const tstring &value, prop_ptr a_this_ptr)
		{
			SetPointer setter=GetterSetterPointer<GetPointer, SetPointer>::get_setter(entry->m_getter_setter);
			Value v;
			accessor a(make_accessor(v));
			a.from_string(value);
			(this_ptr(a_this_ptr)->*setter)(v);
			return 0;
		}
		virtual void get_value(Value &v, const schema_entry *entry, const_prop_ptr a_this_ptr) const
		{
			GetPointer getter=GetterSetterPointer<GetPointer, SetPointer>::get_getter(entry->m_getter_setter);
			v=(this_ptr(a_this_ptr)->*getter)();
		}
		virtual const Value *get_ptr(const schema_entry * /*entry*/, const_prop_ptr /*a_this_ptr*/) const
		{
			return 0;
		}
		virtual Value *get_ptr(const schema_entry * /*entry*/, prop_ptr /*a_this_ptr*/) const
		{
			return 0;
		}
		virtual size_t get_sizeof(const schema_entry *) const
		{
			return 0;
		}
		virtual void set_value(const Value &v, const schema_entry *entry, prop_ptr a_this_ptr)
		{
			SetPointer setter=GetterSetterPointer<GetPointer, SetPointer>::get_setter(entry->m_getter_setter);
			(this_ptr(a_this_ptr)->*setter)(v);
		}

	};

    template <typename AggregateType, typename ValueType>
    void set_value(AggregateType &ag, const char *member_name, const ValueType &t)
    {
        dynamic_cast_accessor<ValueType>(make_aggregate(ag)[member_name]).set(t);
    }
    template <typename AggregateType, typename ValueType>
    ValueType get_value(AggregateType &ag, const char *member_name)
    {
        dynamic_cast_accessor<ValueType>(make_const_aggregate(ag)[member_name]).get<ValueType>();
    }

	/** converter class calling external (non-member) functions to get or set a value. */
	template <class Value, class PropClass, class GetPointer, class SetPointer>
	class converter_with_ext_getset:public converter_value_base<Value>
	{
		inline const PropClass *this_ptr(const_prop_ptr aPtr) const
		{
			return static_cast<const PropClass*>(aPtr);
		}
		inline PropClass *this_ptr(prop_ptr aPtr)
		{
			return static_cast<PropClass*>(aPtr);
		}
		prop_t  m_actual_type;
	public:
		converter_with_ext_getset(const string &type_name="")
			:converter_value_base<Value>(type_name, 0), m_actual_type(get_prop_type<Value>())
		{
		}
		string get_type_name() const { return m_actual_type->get_type_name(); }
		tstring to_string(const schema_entry *entry, const_prop_ptr a_this_ptr)
		{
			Value v;
			get_value(v, entry, a_this_ptr);
			const_accessor a(make_const_accessor(v));
			return a.to_string();
		}
		size_t from_string(const schema_entry *entry, const tstring &value, prop_ptr a_this_ptr)
		{
			Value v;
			accessor a(make_accessor(v));
			a.from_string(value);
			set_value(v, entry, a_this_ptr);
			return 0;
		}
		void get_value(Value &v, const schema_entry *entry, const_prop_ptr a_this_ptr) const
		{
			GetPointer getter=reinterpret_cast<GetPointer>(entry->m_ext_getter_setter.getter);
			v=(*getter)(*this_ptr(a_this_ptr));
		}
		const Value *get_ptr(const schema_entry *entry, const_prop_ptr a_this_ptr) const
		{
			return 0;
		}
		Value *get_ptr(const schema_entry *entry, prop_ptr a_this_ptr) const
		{
			return 0;
		}
		virtual size_t get_sizeof(const schema_entry *) const
		{
			return 0;
		}
		void set_value(const Value &v, const schema_entry *entry, prop_ptr a_this_ptr)
		{
			SetPointer setter=reinterpret_cast<SetPointer>(entry->m_ext_getter_setter.setter);
			(*setter)(*this_ptr(a_this_ptr), v);
		}
	};

	///@{
	template <class Value, class PROPCLASS, class GetPointer, class SetPointer>
	struct prop_type_gettersetter {
		static prop_t get_gettersetter(GetPointer /*g*/, SetPointer /*s*/)
		{
			static converter_with_getset<Value, PROPCLASS, GetPointer, SetPointer> theObjectWithGetterSetter;
			return &theObjectWithGetterSetter;
		}
	};

	template <class Value, class PROPCLASS, class GetPointer, class SetPointer>
	inline prop_t get_prop_type_getter_setter(GetPointer _get, SetPointer _set)
	{
		return prop_type_gettersetter<Value, PROPCLASS, GetPointer, SetPointer>::get_gettersetter(_get, _set);
	}
	///@}

	template <class Value, class PROPCLASS, class GetPointer, class SetPointer>
	struct prop_type_ext_gettersetter {
		static prop_t get_gettersetter(GetPointer g, SetPointer s)
		{
			static converter_with_ext_getset<Value, PROPCLASS, GetPointer, SetPointer> theObjectWithExtGetterSetter;
			return &theObjectWithExtGetterSetter;
		}
	};

	template <class Value, class PROPCLASS, class GetPointer, class SetPointer>
	inline prop_t get_prop_type_ext_getter_setter(GetPointer _get, SetPointer _set)
	{
		return prop_type_ext_gettersetter<Value, PROPCLASS, GetPointer, SetPointer>::get_gettersetter(_get, _set);
	}



};

#endif

#ifdef _MSC_VER
#pragma once
#pragma warning(pop)
#endif
