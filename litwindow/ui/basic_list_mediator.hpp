#ifndef basic_list_mediator_h__31080910
#define basic_list_mediator_h__31080910

#include "litwindow/tstring.hpp"
#include "litwindow/dataadapter.h"
#include <vector>
#include <iterator>
#include <boost/ref.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind/protect.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_member_function_pointer.hpp>
#include <boost/type_traits/is_member_object_pointer.hpp>
#include <boost/type_traits/is_function.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/type_traits/function_traits.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_reference.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
//#include <boost/archive/text_wiarchive.hpp>
//#include <boost/archive/text_woarchive.hpp>

#include <boost/version.hpp>
#if BOOST_VERSION < 104700
#ifndef BOOST_TYPEOF_SILENT
#define BOOST_TYPEOF_SILENT
#endif
#endif

#include <boost/typeof/typeof.hpp>
#include <iterator>

//#pragma optimize("ty", on)

namespace litwindow {
    namespace ui {
		using namespace std;

		class basic_column_label
		{
		public:
            /// the title or header of the column
            tstring title() const { return m_title; }
            /// set the column width
            void width(int new_width) { m_width=new_width; }
            /// return the column width
            int width() const { return m_width; }
            /// visibility
            bool visible() const { return m_visible; }
            void visible(bool do_show) { m_visible=do_show; }
			bool image() const { return m_image; }
			void image(bool is_image) { m_image=is_image; }
			void position(int new_position) { m_position=new_position; }
			int  position() const { return m_position; }
			template <typename Archive>
			void serialize(Archive &ar, const unsigned int version)
			{
				ar & BOOST_SERIALIZATION_NVP(m_title) & BOOST_SERIALIZATION_NVP(m_width) & BOOST_SERIALIZATION_NVP(m_visible);
				if (version>=1)
					ar & BOOST_SERIALIZATION_NVP(m_position);
				else if (Archive::is_loading())
					m_position=-1;
			}
			basic_column_label(const tstring &title, int width=-1, bool visible=true, bool is_image=false)
				:m_title(title),m_width(width),m_visible(visible),m_image(is_image),m_position(-1){}
		protected:
			tstring m_title;
			int     m_width;
			bool    m_visible;
			bool	m_image;
			int		m_position;
		};


        template <typename Value/*, typename TextRenderer*/>
		class basic_column_descriptor:public basic_column_label
        {
        public:
            typedef boost::function<void (const Value&, tstring&)> text_renderer_type;
			typedef boost::function<void (const Value&, int&)> image_index_renderer_type;
			typedef boost::function<bool (const Value &left, const Value &right)> comparator_type;
            //typedef TextRenderer text_renderer_type;
            typedef Value value_type;
        public:
			text_renderer_type m_text_renderer;
			image_index_renderer_type m_image_index_renderer;
			comparator_type m_comparator;
			basic_column_descriptor(const tstring &title, int width=-1, const text_renderer_type &v=text_renderer_type())
                :basic_column_label(title, width, true, false), m_text_renderer(v), m_image_index_renderer(0)
            {}
			basic_column_descriptor(const tstring &title, int width, const image_index_renderer_type &i)
				:basic_column_label(title, width, true, true), m_text_renderer(0), m_image_index_renderer(i)
			{}

			basic_column_descriptor():basic_column_label(tstring(), -1, false, false){}
            template <typename Control>
            bool render_element(size_t row, size_t col, Control &c, const value_type &v) const { return false; }

            bool render_element(tstring &c, const value_type &v) const
            {
				if (m_text_renderer)
	                m_text_renderer(v, c);
                return true; 
            }
			bool render_element_image(int &i, const value_type &v) const
			{
				if (m_image_index_renderer)
					m_image_index_renderer(v, i);
				else
					i=-1;
				return true;
			}

            //tstring value_as_text(const typename text_renderer_type::value_type &t) const;// { return m_text_renderer(t); }
			bool compare(const value_type &left, const value_type &right, size_t col) const
			{
				if (m_comparator)
					return m_comparator(left, right);
				if (m_image_index_renderer) {
					int l, r;
					m_image_index_renderer(left, l);
					m_image_index_renderer(right, r);
					return l<r;
				}
				tstring left_string, right_string;
				render_element(left_string, left);
				render_element(right_string, right);
				return left_string<right_string;
			}
            template <typename Archive>
            void serialize(Archive &ar, const unsigned int version)
            {
				ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(basic_column_label);				
            }
		};

		template <typename Value>
		basic_column_descriptor<Value> image_column(const tstring &title, int width, const typename basic_column_descriptor<Value>::image_index_renderer_type &i)
		{
			return basic_column_descriptor<Value>(title, width, i);
		}

#pragma region TextRenderer
        template <typename Value>
        inline void to_string(Value v, tstring &c)
        {
            //c=boost::lexical_cast<tstring>(v);
            c=litwindow::make_const_accessor(v).to_string();
        }
        template <typename ValueType, typename MemberType>
        inline const MemberType &to_member(const ValueType &v, MemberType (ValueType::*ptr_to_member))
        {
            return v.*ptr_to_member;
        }
        template <typename MemberType, typename ValueType>
        inline typename basic_column_descriptor<ValueType>::text_renderer_type make_text_renderer(MemberType (ValueType::*ptr_to_member))
        {
            return boost::bind(&to_string<MemberType>,
                boost::bind(&to_member<ValueType, MemberType>, _1, ptr_to_member),
                _2);
        }
		template <typename MemberType, typename ValueType, typename Formatter>
		inline typename basic_column_descriptor<ValueType>::text_renderer_type make_text_renderer(MemberType (ValueType::*ptr_to_member), Formatter fmt)
		{
			using boost::bind;
			return bind(&to_string<typename Formatter::result_type>,
					bind(fmt, bind(&to_member<ValueType, MemberType>, _1, ptr_to_member) ),
				_2);
		}
		template <typename Result, typename MemberClass>
		inline void member_result_to_string(Result (MemberClass::*ptr_to_member)() const, const MemberClass &p, tstring &c)
		{
			to_string((p.*ptr_to_member)(), c);
		}
        template <typename MemberType, typename ValueType>
        inline typename basic_column_descriptor<ValueType>::text_renderer_type make_text_renderer(MemberType (ValueType::*ptr_to_member)() const)
        {
			using namespace boost;
			function<void (ValueType, tstring &c)> render(bind(&member_result_to_string<MemberType, ValueType>, ptr_to_member, _1, _2));
			return render;
        }

		template <typename MemberType, typename ValueType>
		inline bool comparator(const ValueType &left, const ValueType &right, MemberType (ValueType::*ptr_to_member))
		{
			return (left.*ptr_to_member)<(right.*ptr_to_member);
		}
		template <typename MemberType, typename ValueType>
		inline bool compare_memfnc(const ValueType &left, const ValueType &right, MemberType (ValueType::*ptr_to_member)() const)
		{
			return (left.*ptr_to_member)()<(right.*ptr_to_member)();
		}
#ifdef not
		template <typename MemberType, typename ValueType>
		inline basic_column_descriptor<ValueType> make_column_descriptor(const wstring &title, int width, MemberType (ValueType::*ptr_to_member))
		{
			basic_column_descriptor<ValueType> rc(title, width, make_text_renderer(ptr_to_member));
			//function<const MemberType&(const ValueType &, MemberType (ValueType::*))> left, right;
			//left=bind(&to_member<ValueType, MemberType>, _1, ptr_to_member);
			//right=bind(&to_member<ValueType, MemberType>, _1, ptr_to_member);
			rc.m_comparator=boost::bind(&comparator<MemberType, ValueType>, _1, _2, ptr_to_member);
				//bind(std::less<MemberType>(), bind(&to_member<ValueType, MemberType>, _1, ptr_to_member), bind(to_member<ValueType, MemberType>,_2, ptr_to_member));
			return rc;
		}
#endif // _DEBUG
#ifdef not
		template <typename MemberFnc, typename ValueType>
		inline basic_column_descriptor<ValueType> make_column_descriptor(const wstring &title, int width, MemberFnc (ValueType::*ptr_to_member)() const)
		{
			basic_column_descriptor<ValueType> rc(title, width, make_text_renderer(ptr_to_member));
			function<MemberFnc (const ValueType &)> left, right;
			left=bind(ptr_to_member, _1);
			right=bind(ptr_to_member, _1);
			rc.m_comparator=boost::bind(&compare_memfnc<MemberFnc, ValueType>, _1, _2, ptr_to_member);
			//rc.m_comparator=boost::bind(&comparator<MemberFnc, ValueType>, _1, _2, ptr_to_member);
			//rc.m_comparator=boost::bind(std::less<MemberFnc>, left, right);
			return rc;
		}
#endif // not
		template <typename GetterFnc, typename ValueType, typename Formatter>
		inline basic_column_descriptor<ValueType> make_column_descriptor_with_fmt(const wstring &title, int width, const boost::function<GetterFnc(const ValueType &)> &ptr, Formatter fmt)
		{
			using boost::bind;
			basic_column_descriptor<ValueType> rc(title, width, bind(fmt, bind(ptr, _1), _2));
			rc.m_comparator=bind(less<GetterFnc>(), bind(ptr, _1), bind(ptr, _2));
			return rc;
		}

#pragma endregion TextRenderer

		template <typename Accessor, typename RowValue, typename Enabled=void>
		struct column_accessor
		{
			//typedef Accessor accessor_type;
			//typedef typename accessor_type::result_type column_value_type;
		};
		template <typename ColValue, typename RowValue>
		struct column_accessor<ColValue (RowValue::*), RowValue>
		{
			typedef ColValue column_value_type;
			typedef RowValue row_value_type;
			typedef void (formatter_type)(column_value_type, wstring &);
		};
		template <typename ColValue, typename RowValue>
		struct column_accessor<ColValue (RowValue::*)()const, RowValue>
		{
			typedef ColValue column_value_type;
			typedef RowValue row_value_type;
			typedef void (formatter_type)(column_value_type, wstring &);
		};
		template <typename ColValue, typename RowValue>
		struct column_accessor<ColValue (*)(const RowValue&), RowValue>
		{
			typedef ColValue column_value_type;
			typedef RowValue row_value_type;
			typedef void (formatter_type)(const column_value_type&, wstring &);
		};
		template <typename ColumnAccessor, typename RowValue>
		struct column_accessor<ColumnAccessor, RowValue, typename boost::disable_if<boost::is_void<typename ColumnAccessor::result_type> >::type >
		{
			//typedef typename boost::remove_const<typename boost::remove_reference<typename ColumnAccessor::result_type>::type>::type column_value_type;
			typedef typename ColumnAccessor::result_type column_value_type;
			typedef RowValue row_value_type;
			typedef void (formatter_type)(const column_value_type&, wstring &);
		};

		template <typename RowValue, typename ColumnAccessor>
		basic_column_descriptor<RowValue> make_basic_column_descriptor(const wstring &title, int width, const ColumnAccessor &a, typename column_accessor<ColumnAccessor, RowValue>::formatter_type f=0)
		{
			typedef typename column_accessor<ColumnAccessor, RowValue>::column_value_type column_value_type;
			//typedef BOOST_TYPEOF(boost::bind(a, _1)(*(const RowValue*)0)) column_value_type;
			basic_column_descriptor<RowValue> rc(title, width);
			if (f==0)
				rc.m_text_renderer=boost::bind(&to_string<column_value_type>, boost::bind(a, _1), _2);
			else
				rc.m_text_renderer=boost::bind(f, boost::bind(a, _1), _2);
			rc.m_comparator=boost::bind(a, _1) < boost::bind(a, _2);
			return rc;
		}

		template <typename RowValue>
		basic_column_descriptor<RowValue> make_basic_column_descriptor(const wstring &title, int width, boost::function<void(const RowValue&, wstring &)> fncObject)
		{
			//typedef BOOST_TYPEOF(boost::bind(a, _1)(*(const RowValue*)0)) column_value_type;
			basic_column_descriptor<RowValue> rc(title, width);
			rc.m_text_renderer=boost::bind(fncObject, _1, _2);
			return rc;
		}

		template <typename RowValue, typename R>
		basic_column_descriptor<RowValue> make_basic_column_descriptor(const wstring &title, int width, R (RowValue::*r), typename column_accessor<R (RowValue::*), RowValue>::formatter_type f)
		{
			basic_column_descriptor<RowValue> rc(title, width);
			typedef typename boost::remove_reference<R>::type column_value_type;
			if (f==0)
				rc.m_text_renderer=boost::bind(&to_string<column_value_type>, boost::bind(r, _1), _2);
			else
				rc.m_text_renderer=boost::bind(f, boost::bind(r, _1), _2);
			rc.m_comparator=boost::bind(r, _1) < boost::bind(r, _2);
			return rc;
		}

		template <typename RowValue, typename R>
		basic_column_descriptor<RowValue> make_basic_column_descriptor(const wstring &title, int width, R (RowValue::*r)() const, typename column_accessor<R (RowValue::*)(), RowValue>::formatter_type f)
		{
			basic_column_descriptor<RowValue> rc(title, width);
			typedef typename boost::remove_reference<R>::type column_value_type;
			if (f==0)
				rc.m_text_renderer=boost::bind(&to_string<column_value_type>, boost::bind(r, _1), _2);
			else
				rc.m_text_renderer=boost::bind(f, boost::bind(r, _1), _2);
				rc.m_text_renderer=boost::bind(&to_string<column_value_type>, boost::bind(r, _1), _2);
			rc.m_comparator=boost::bind(r, _1) < boost::bind(r, _2);
			return rc;
		}

		//template <typename RowValue, typename TextRenderer>
		//basic_column_descriptor<RowValue> make_basic_column_descriptor(const wstring &title, int width, void (TextRenderer fncObject)(const RowValue&, wstring &))
		//{
		//	//typedef BOOST_TYPEOF(boost::bind(a, _1)(*(const RowValue*)0)) column_value_type;
		//	basic_column_descriptor<RowValue> rc(title, width);
		//	rc.m_text_renderer=boost::bind(fncObject, _1, _2);
		//	return rc;
		//}

		template <typename ColumnDescriptor>
        class basic_columns_adapter
        {
        public:
            typedef ColumnDescriptor column_descriptor_type;
            typedef typename column_descriptor_type::value_type value_type;
            typedef typename column_descriptor_type::text_renderer_type text_renderer_type;
			typedef typename column_descriptor_type::image_index_renderer_type image_index_renderer_type;

            typedef std::vector<column_descriptor_type> columns_t;
            const columns_t &columns() const { return m_column_data; }
			columns_t &columns() { return m_column_data; }
            std::back_insert_iterator<columns_t> do_add(const column_descriptor_type &d)
            {
                set_dirty();
                m_column_data.push_back(d);
                return std::back_inserter<columns_t>(m_column_data);
            }
			struct default_formatter
			{
			};
            struct back_inserter
            {
			private:
                basic_columns_adapter *m_c;
				void set_this(basic_columns_adapter *c) { m_c=c; }
				friend class basic_columns_adapter;
			public:
				back_inserter add(const column_descriptor_type &d) const
				{
					m_c->do_add(d);
					return *this;
				}
				template <typename ColDescrType>
                back_inserter operator()(const ColDescrType &d) const
				{ 
                    return add(d);
                }

				back_inserter operator()(const wstring &title, int width, void (fnc)(const value_type&, wstring&)) const
				{
					return operator()(column_descriptor_type(title, width, text_renderer_type(fnc)));
				}
				//back_inserter operator()(const tstring &title, int width, const text_renderer_type &a) const
				//{
				//	return operator()(column_descriptor_type(title, width, a));
				//}
				template <typename ValueMember>
				back_inserter operator()(const tstring &title, int width, ValueMember (*free_function)(const value_type &), typename column_accessor<ValueMember (*)(const value_type &), value_type>::formatter_type f=0) const
				{
					return add(make_basic_column_descriptor<value_type>(title, width, free_function, f));
				}
				template <typename ValueMember>
				back_inserter operator()(const tstring &title, int width, ValueMember (value_type::*ptr_to_member), typename column_accessor<ValueMember (value_type::*), value_type>::formatter_type f=0) const
				{
					return add(make_basic_column_descriptor(title, width, ptr_to_member, f));
				}
				template <typename ValueMember>
				back_inserter operator()(const tstring &title, int width, ValueMember (value_type::*ptr_mem_fnc)() const, typename column_accessor<ValueMember (value_type::*)() const, value_type>::formatter_type f=0) const
				{
					return add(make_basic_column_descriptor<value_type>(title, width, ptr_mem_fnc, f));
				}
				template <typename T1>
				back_inserter operator()(const tstring &title, int width, const T1 &v, typename column_accessor<T1, value_type>::formatter_type f=0, typename boost::disable_if<boost::is_void<typename T1::result_type> >::type *dmy=0) const
				{
					return add(make_basic_column_descriptor<value_type>(title, width, v, f));
				}
				template <typename TextRenderer>
				back_inserter operator()(const wstring &title, int width, TextRenderer t, typename boost::enable_if<boost::is_void<typename TextRenderer::result_type> >::type* dmy=0) const
				{
					return add(column_descriptor_type(title, width, text_renderer_type(t)));
				}
            };
			back_inserter add;
			//template <typename T1, typename T2> back_inserter add(const tstring &title, int width, const T1 &t1, const T2 &t2) { return back_inserter(*this)(title, width, t1, t2); }
			//template <typename T1> back_inserter add(const tstring &title, int width, const T1 &t1) { return back_inserter(*this)(title, width, t1); }
			//template <typename T1> back_inserter add(const tstring &title, int width, T1 (value_type::*t1)) { return back_inserter(*this)(title, width, t1); }
			//back_inserter add(const column_descriptor_type &d) { return back_inserter(*this)(d); }
            size_t size() const { return columns().size(); }
			bool empty() const { return columns().empty(); }
            bool dirty() const { return m_dirty; }
			void dirty(bool is_dirty) { m_dirty=is_dirty; }
			void clear() { m_column_data.clear(); dirty(true); }
            basic_columns_adapter()
                :m_dirty(true){ add.set_this(this); }
            const column_descriptor_type &column_descriptor(size_t idx) const { return columns().at(idx); }
			column_descriptor_type &column_descriptor(size_t idx) { return columns().at(idx); }

			size_t get_column_index(const wstring &title) const
			{
				columns_t::const_iterator i=find_if(columns().begin(), columns().end(), boost::bind(&columns_t::value_type::title, _1)==title);
				if (i==columns().end())
					throw runtime_error("get_column_index: no such column");
				return i-columns().begin();
			}

            template <typename Control>
            bool render_element_at(size_t row, size_t col, Control &c, const value_type &v) const
            {
                return columns().at(col).render_element(row, col, c, v);
            }
            bool render_element_at(size_t col, tstring &rc, const value_type &v) const
            {
                return columns().at(col).render_element(rc, v);
            }
			bool render_element_image_at(size_t col, int &rc, const value_type &v) const
			{
				return columns().at(col).render_element_image(rc, v);
			}
			bool render_element_at(const wstring &column, tstring &rc, const value_type &v) const
			{
				return render_element_at(get_column_index(column), rc, v);
			}
            typename columns_t::value_type &at(size_t col) { return columns().at(col); }
			typename const columns_t::value_type &at(size_t col) const { return columns().at(col); }
            //const value_type &column_description(size_t idx) const { return columns().at(idx); }

            //const element_value_type &get(const value_type &e, const column_position_type &pos) const
            //{
            //    return m_element_adapter(e, pos);
            //}
			void set_dirty() { dirty(true); }
			void clear_dirty() { dirty(false); }
			void toggle_show(size_t col)
			{
				columns().at(col).visible(!columns().at(col).visible());
				dirty(true);
			}

			bool compare(size_t col, const typename column_descriptor_type::value_type &left, const typename column_descriptor_type::value_type &right) const
			{
				return at(col).compare(left, right, col);
			}
	
            template <typename Archive>
            void serialize(Archive &ar, const unsigned int version)
            {
				if (Archive::is_loading()) {
					columns_t temp;
					ar & boost::serialization::make_nvp("m_column_data", temp);
					merge_in(temp);
				} else {
					ar & boost::serialization::make_nvp("m_column_data", m_column_data);
				}
            }
		protected:
        private:
            bool m_dirty;
            columns_t m_column_data;

			void merge_in(const columns_t &in)
			{
				struct merge_element
				{
					static void do_merge(columns_t &dest, const columns_t::value_type &src)
					{
						columns_t::iterator i;
						i=std::find_if(dest.begin(), dest.end(), boost::bind(&columns_t::value_type::title, _1) == src.title());
						if (i!=dest.end()) {
							columns_t::value_type &current(*i);
							current.width(src.width());
							current.visible(src.visible());
							current.position(src.position());
						}
					}
				};
				for_each(in.begin(), in.end(), boost::bind(&merge_element::do_merge, boost::ref(m_column_data), _1));
				m_dirty=true;
			}
            //element_adapter m_element_adapter;
        };

    }
}


namespace litwindow {
    namespace ui {

        template <typename DatasetAdapter>
        class basic_dataset_accessor
        {
        public:
            typedef DatasetAdapter value_type;
            typedef typename DatasetAdapter::value_type element_type;

            void sort();
            void filter();
            size_t size() const;
        };

        template <typename ContainerType, typename ItemHandle=typename ContainerType::iterator>
        class stl_container_dataset_accessor
        {
        public:
            typedef stl_container_dataset_accessor _Myt;
            typedef ContainerType container_type;
            typedef typename ContainerType::value_type value_type;
            typedef ItemHandle item_handle_type;
            typedef std::vector<item_handle_type> sorted_container_type;
            typedef boost::function<bool(const item_handle_type &left, const item_handle_type &right)> sort_pred_type;
            typedef boost::function<bool(const item_handle_type &f)> filter_pred_type;
        private:
            container_type *m_c;
            sorted_container_type m_item_handles;
            //columns_adapter_type m_columns_adapter;
            sort_pred_type m_sort_pred;
            filter_pred_type m_filter_pred;
            bool m_needs_update;

            container_type &container() const { return *m_c; }
        public:
            //const columns_adapter_type &columns_adapter() const { return m_columns_adapter; }
            //columns_adapter_type &columns_adapter() { return m_columns_adapter; }
            _Myt &set_container(container_type &v)
            {
                m_c=&v;
                m_needs_update=true;
                return *this;
            }
            //_Myt &set_columns_adapter(const columns_adapter_type &cadapter)
            //{
            //    m_columns_adapter=cadapter;
            //    return *this;
            //}
            size_t size() const { return m_item_handles.size(); }
            void sort()
            {
                sorted_container_type::iterator dest=m_item_handles.begin();
                container_type::iterator i=container().begin();
                while (dest!=m_item_handles.end() && i!=container().end()) {
                    if (!m_filter_pred || m_filter_pred(i))
                        *dest++=i;
                    ++i;
                }
                if (dest<m_item_handles.end())
                    m_item_handles.erase(dest, m_item_handles.end());
                else while (i!=container().end()) {
                    if (!m_filter_pred || m_filter_pred(i))
                        m_item_handles.push_back(i);
                    ++i;
                }
                if (m_sort_pred)
                    std::sort(m_item_handles.begin(), m_item_handles.end(), m_sort_pred);
            }
			void set_dirty() { m_needs_update=true; }
            void force_update() { m_needs_update=true; update(); }
            void update() { if (m_needs_update) sort(); m_needs_update=false; }
            void refresh(bool force=false) { if (force) force_update(); else update(); }
            const value_type &value_at(size_t pos) const
			{
				//TODO: this needs a mechanism to return 'no-item' to signal the caller
				// that the container has changed and no longer has 'pos' items
				return *m_item_handles.at(pos); 
			}
            void modify_value_at(size_t pos, const value_type &new_value)
            {
                *m_item_handles.at(pos)=new_value;
                m_needs_update=true;
            }
            void delete_value_at(size_t pos)
            {
                (*m_c).erase(m_item_handles.at(pos));
                m_needs_update=true;
            }
            void clear()
            {
                (*m_c).clear();
                m_needs_update=true;
            }
            void append(const value_type &new_value)
            {
                std::back_inserter(*m_c)=new_value;
                m_needs_update=true;
            }
        };


        template <typename ContainerType>
        inline stl_container_dataset_accessor<ContainerType> make_dataset_adapter(ContainerType &d)
        {
            stl_container_dataset_accessor<ContainerType> rc;
            rc.set_container(d);
            return rc;
        }

        template <typename ColumnTraits, typename UIControl, typename Columns>
        void setup_columns(ColumnTraits tr, UIControl c, Columns &cols)
        {
            size_t idx=0;
            while (idx<cols.size()) {
                if (idx>=tr.count(c))
                    tr.insert_column(c, idx, cols.column_descriptor(idx));
                else
                    tr.set_column(c, idx, cols.column_descriptor(idx));
                ++idx;
            }
            while (tr.count(c)>cols.size())
                tr.remove_column(c, tr.count(c)-1);
        }

        class basic_ui_control_adapter
        {
        public:
            void begin_update() {}
            void end_update() {}
            template <typename Mediator>
            void connect_mediator(const Mediator &m) {}
            static const size_t npos=(size_t)-1;
            size_t get_selected_index() const { return npos; }
        };

//////////////////////////////////////////////////////////////////////////

        /*! A basic_list_mediator encapsules all interactions between a tabular UI control
        * and a dataset. The DatasetAdapter connects the mediator with a dataset
        * structure such as a vector, odbc statement or others. The UIControlAdapter
        * connects the mediator with a UI control such as a list, combobox, listctrl,
        * grid or others.
        */
		template <typename DatasetAdapter, typename UIControlAdapter, typename ColumnsAdapter=typename DatasetAdapter::columns_adapter_type>
        class basic_list_mediator
        {
        public:
            /// refresh the ui control
            /// call this after the dataset has changed
            void refresh();

			typedef basic_list_mediator _Myt;
            typedef UIControlAdapter ui_control_adapter_type;
            typedef DatasetAdapter dataset_adapter_type;
            typedef ColumnsAdapter columns_adapter_type;
            typedef typename dataset_adapter_type::value_type value_type;

            //void set_columns_adapter(const columns_adapter_type &cols) { m_columns=cols; m_needs_refresh_columns=true; }
            columns_adapter_type &columns_adapter()
			{ 
				return m_columns_adapter;
				//return m_dataset_adapter.columns_adapter(); 
			}
            const columns_adapter_type &columns_adapter() const { return m_columns_adapter; }

			columns_adapter_type &columns() { return columns_adapter(); }
			const columns_adapter_type &columns() const { return columns_adapter(); }
            _Myt &set_columns_adapter(const columns_adapter_type &cadapter)
            {
                m_columns_adapter=cadapter;
                return *this;
            }

            template <typename Data, typename Columns>
            void set_dataset(Data &d, const Columns &c)
            {
                set_dataset_adapter(make_dataset_adapter(d));
				set_columns_adapter(c);
            }
            template <typename List>
            void set_ui(List &l)
            {
                set_ui_adapter(make_list_adapter(l));
            }
            void set_ui_adapter(const ui_control_adapter_type &a)
            { 
                m_ui_control_adapter=a;
                m_ui_control_adapter.connect_mediator(*this);
            }

            void set_dataset_adapter(const dataset_adapter_type &d) { m_dataset_adapter=d; }
            const dataset_adapter_type &dataset_adapter() const { return m_dataset_adapter; }
            dataset_adapter_type &dataset_adapter() { return m_dataset_adapter; }
            const ui_control_adapter_type &ui_adapter() const { return m_ui_control_adapter; }
            ui_control_adapter_type &ui_adapter() { return m_ui_control_adapter; }

            basic_list_mediator()
                :m_needs_refresh_columns(true){}

            void get_item_text(size_t row, size_t col, tstring &rc) const
            {
                const columns_adapter_type &ca(columns_adapter());
                ca.render_element_at(col, rc, dataset_adapter().value_at(row));
            }

            void refresh_dataset()
            {
                m_dataset_adapter.refresh(true);
            }

            const value_type &get_selected_item() const
            {
                return dataset_adapter().value_at(ui_adapter().get_selected_index());
            }
            void modify_selected_item(const value_type &data)
            {
                dataset_adapter().modify_value_at(ui_adapter().get_selected_index(), data);
            }
            void delete_selected_item()
            {
                dataset_adapter().delete_value_at(ui_adapter().get_selected_index());
            }
            void delete_all_items()
            {
                dataset_adapter().clear();
            }
            void append_item(const value_type &data)
            {
                dataset_adapter().append(data);
            }

#ifdef not
			template <typename Fnc>
			void visit(size_t idx, Fnc f)
			{
				f(dataset_adapter().value_at(idx), idx);
			}
			template <typename Fnc>
			void for_each_selected(Fnc f)
			{
				ui_adapter().for_each_selected(bind(&basic_list_mediator::visit, this, _1, f));
			}
#endif // not
			template <typename ResultSet, typename Fnc>
			void visit(ResultSet* rc, size_t idx, Fnc f)
			{
				rc->push_back(f(dataset_adapter().value_at(idx), idx));
			}
			template <typename Fnc>
			std::vector<typename boost::function_traits<Fnc>::result_type> get_selection(Fnc f)
			{
				std::vector<typename boost::function_traits<Fnc>::result_type> rc;
				ui_adapter().for_each_selected(bind(&basic_list_mediator::visit, this, &rc, _1, f));
				return rc;
			}
			void show_hide_column(int col, bool show_hide);
			void toggle_show_column(int col);

        protected:
            void refresh_columns(bool do_refresh=true);
            void refresh_list();
            
            void begin_update() { m_ui_control_adapter.begin_update(); }
            void end_update() { m_ui_control_adapter.end_update(); }

            void setup_columns();

            ui_control_adapter_type m_ui_control_adapter;
            columns_adapter_type    m_columns_adapter;
            dataset_adapter_type   m_dataset_adapter;
            bool            m_needs_refresh_columns;
        };

		//------------------------------------------------------------------------------------------------------------------------------------
		//////////////////////////////////////////////////////////////////////////
        template <typename DatasetAdapter, typename UIControlAdapter, typename ColumnsAdapter>
        void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter, typename ColumnsAdapter>::refresh()
        {
            begin_update();
            refresh_dataset();
            refresh_columns(m_needs_refresh_columns);
            refresh_list();
            end_update();
        }

        template <typename DatasetAdapter, typename UIControlAdapter, typename ColumnsAdapter>
        void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter, typename ColumnsAdapter>::refresh_columns(bool do_refresh)
        {
            if (do_refresh || columns_adapter().dirty()) {
                begin_update();
                setup_columns();
                end_update();
                m_needs_refresh_columns=false;
            }
        }

		template <typename DatasetAdapter, typename UIControlAdapter, typename ColumnsAdapter>
		void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter, ColumnsAdapter>::toggle_show_column(int col)
		{
			columns_adapter().toggle_show(col);
			refresh_columns(true);
		}

        template <typename DatasetAdapter, typename UIControlAdapter, typename ColumnsAdapter>
        void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter, typename ColumnsAdapter>::refresh_list()
        {
            m_ui_control_adapter.refresh_list(m_dataset_adapter, m_columns_adapter);
        }

        template <typename DatasetAdapter, typename UIControlAdapter, typename ColumnsAdapter>
        void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter, typename ColumnsAdapter>::setup_columns()
        {
            m_ui_control_adapter.setup_columns(columns_adapter());
			columns_adapter().clear_dirty();
        }

    }
}

BOOST_CLASS_VERSION(litwindow::ui::basic_column_label, 1);

#pragma optimize("", on)
#endif // list_mediator_h__31080910
