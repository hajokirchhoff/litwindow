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
            int width() const { return m_width<0 ? -m_width : m_width; }
            /// visibility
            bool visible() const { return m_visible; }
            void visible(bool do_show) { m_visible=do_show; }
			bool image() const { return m_image; }
			void image(bool is_image) { m_image=is_image; }
			void position(int new_position) { m_position=new_position; }
			int  position() const { return m_position; }

			enum col_alignment {
				left = 0,
				right = 1,
				center = 2
			};
			col_alignment alignment() const { return m_align; }
			void alignment(col_alignment new_align) { m_align = new_align; }

			template <typename Archive>
			void serialize(Archive &ar, const unsigned int version)
			{
				ar & BOOST_SERIALIZATION_NVP(m_title) & BOOST_SERIALIZATION_NVP(m_width) & BOOST_SERIALIZATION_NVP(m_visible);
				if (version >= 1)
					ar & BOOST_SERIALIZATION_NVP(m_position);
				else if (Archive::is_loading())
					m_position = -1;
				if (version >= 2)
					ar & BOOST_SERIALIZATION_NVP(m_align);
				else {
					if (Archive::is_loading()) {
						if (m_width < 0) {
							m_align = right;
							m_width = -m_width;
						}
						else
							m_align = left;
					}
				}
			}
			basic_column_label(const tstring &title, int width=-1, bool visible=true, bool is_image=false)
				:m_title(title),m_width(width),m_visible(visible),m_image(is_image),m_position(-1),m_align(width<-1 ? right : left){}
		protected:
			tstring m_title;
			int     m_width;
			bool    m_visible;
			bool	m_image;
			int		m_position;
			col_alignment m_align = left;
		};

		template <typename Value>
		inline void to_string(Value v, tstring& c)
		{
			//c=boost::lexical_cast<tstring>(v);
			c = litwindow::make_const_accessor(v).to_string();
		}

        template <typename Value/*, typename TextRenderer*/>
		class basic_column_descriptor:public basic_column_label
        {
        public:
            using text_renderer_type = boost::function<void (const Value&, tstring&)>;
			using image_index_renderer_type = boost::function<void (const Value&, int&)>;
			using comparator_type = boost::function<bool (const Value &left, const Value &right)>;

            using value_type = Value;
        public:
			text_renderer_type m_text_renderer;
			image_index_renderer_type m_image_index_renderer;
			comparator_type m_comparator;

			basic_column_descriptor(const tstring &title, int width)
				:basic_column_label(title, width)
			{}
/*
			template <typename Accessor, typename Formatter>
			basic_column_descriptor(const tstring& title, int width, boost::function<Accessor(const Value&)> acc, const Formatter& fmt)
				: basic_column_descriptor(title, width)
			{
				m_text_renderer = boost::bind(fmt, boost::bind(acc, boost::placeholders::_1), boost::placeholders::_2);
				m_comparator = boost::bind(acc, boost::placeholders::_1) < boost::bind(acc, boost::placeholders::_2);
			}
*/
/*
			template <typename Accessor>
			basic_column_descriptor(const tstring &title, int width, Accessor acc)
				: basic_column_descriptor(title, width)
			{
				using ColType = typename std::result_of<Accessor(value_type)>::type;
				m_text_renderer = boost::bind(&to_string<ColType>, acc, boost::placeholders::_1);
			}
*/

			///! Constructor for accessor functor
			template <typename Accessor, typename ValueType = value_type, typename ColumnType = std::result_of<Accessor(const ValueType&)>::type>
			basic_column_descriptor(const tstring &title, int width, Accessor acc)
				:basic_column_label(title, width)
			{
				m_text_renderer = boost::bind<void>(&to_string<ColumnType>, boost::bind<ColumnType>(acc, boost::placeholders::_1), boost::placeholders::_2);
				m_comparator = boost::bind<ColumnType>(acc, boost::placeholders::_1) < boost::bind<ColumnType>(acc, boost::placeholders::_2);
			}
			///! Constructor for accessor functor with separate formatter
			template <typename Accessor, typename Formatter, typename ValueType = value_type, typename ColumnType = std::result_of<Accessor(const ValueType&)>::type>
			basic_column_descriptor(const tstring &title, int width, Accessor acc, const Formatter &fmt)
				:basic_column_label(title, width)
			{
				m_text_renderer = boost::bind<void>(fmt, boost::bind<ColumnType>(acc, boost::placeholders::_1), boost::placeholders::_2);
				m_comparator = boost::bind<ColumnType>(acc, boost::placeholders::_1) < boost::bind<ColumnType>(acc, boost::placeholders::_2);
			}

			///! Constructor for text renderer functor
			template <typename Accessor, typename std::enable_if<std::is_void<typename std::result_of<Accessor(const value_type &, wstring&)>::type>::value, int>::type = 0 >
			basic_column_descriptor(const tstring &title, int width, Accessor acc)
				: basic_column_label(title, width)
			{
				m_text_renderer = acc;// boost::bind<void>(acc, boost::placeholders::_1, boost::placeholders::_2);
			}

			///! Constructor for pointer to member function
			template <typename ColumnType>
			basic_column_descriptor(const tstring &title, int width, ColumnType (Value::*acc)() const)
				:basic_column_label(title, width)
			{
				m_text_renderer = boost::bind(&to_string<ColumnType>, boost::bind<ColumnType>(acc, boost::placeholders::_1), boost::placeholders::_2);
				m_comparator = boost::bind<ColumnType>(acc, boost::placeholders::_1) < boost::bind<ColumnType>(acc, boost::placeholders::_2);
			}
			///! Constructor for pointer to free function
			template <typename ColumnType>
			basic_column_descriptor(const tstring &title, int width, ColumnType (*acc)(const Value&))
				: basic_column_label(title, width)
			{
				m_text_renderer = boost::bind(&to_string<ColumnType>, boost::bind<ColumnType>(acc, boost::placeholders::_1), boost::placeholders::_2);
				m_comparator = boost::bind<ColumnType>(acc, boost::placeholders::_1) < boost::bind<ColumnType>(acc, boost::placeholders::_2);
			}
			///! Constructor for pointer to boost::function
			template <typename ColumnType>
			basic_column_descriptor(const tstring &title, int width, boost::function<ColumnType(const Value&)> acc)
				: basic_column_label(title, width)
			{
				m_text_renderer = boost::bind(&to_string<ColumnType>, boost::bind<ColumnType>(acc, boost::placeholders::_1), boost::placeholders::_2);
				m_comparator = boost::bind<ColumnType>(acc, boost::placeholders::_1) < boost::bind<ColumnType>(acc, boost::placeholders::_2);
			}

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
			auto rc = basic_column_descriptor<Value>(title, width);
			rc.m_image_index_renderer = i;
			return rc;
		}

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
			//typedef BOOST_TYPEOF(boost::bind(a, boost::placeholders::_1)(*(const RowValue*)0)) column_value_type;
			basic_column_descriptor<RowValue> rc(title, width);
			if (f==0)
				rc.m_text_renderer=boost::bind(&to_string<column_value_type>, boost::bind(a, boost::placeholders::_1), boost::placeholders::_2);
			else
				rc.m_text_renderer=boost::bind(f, boost::bind(a, boost::placeholders::_1), boost::placeholders::_2);
			rc.m_comparator=boost::bind(a, boost::placeholders::_1) < boost::bind(a, boost::placeholders::_2);
			return rc;
		}

		template <typename RowValue>
		basic_column_descriptor<RowValue> make_basic_column_descriptor(const wstring &title, int width, boost::function<void(const RowValue&, wstring &)> fncObject)
		{
			//typedef BOOST_TYPEOF(boost::bind(a, boost::placeholders::_1)(*(const RowValue*)0)) column_value_type;
			basic_column_descriptor<RowValue> rc(title, width);
			rc.m_text_renderer=boost::bind(fncObject, boost::placeholders::_1, boost::placeholders::_2);
			return rc;
		}

		template <typename RowValue, typename R>
		basic_column_descriptor<RowValue> make_basic_column_descriptor(const wstring &title, int width, R (RowValue::*r), typename column_accessor<R (RowValue::*), RowValue>::formatter_type f)
		{
			basic_column_descriptor<RowValue> rc(title, width);
			typedef typename boost::remove_reference<R>::type column_value_type;
			if (f==0)
				rc.m_text_renderer=boost::bind(&to_string<column_value_type>, boost::bind(r, boost::placeholders::_1), boost::placeholders::_2);
			else
				rc.m_text_renderer=boost::bind(f, boost::bind(r, boost::placeholders::_1), boost::placeholders::_2);
			rc.m_comparator=boost::bind(r, boost::placeholders::_1) < boost::bind(r, boost::placeholders::_2);
			return rc;
		}

		template <typename RowValue, typename R>
		basic_column_descriptor<RowValue> make_basic_column_descriptor_memfnc(const wstring &title, int width, R (RowValue::*r)() const, typename column_accessor<R (RowValue::*)() const, RowValue>::formatter_type f)
		{
			basic_column_descriptor<RowValue> rc(title, width);
			typedef typename boost::remove_reference<R>::type column_value_type;
			if (f==0)
				rc.m_text_renderer=boost::bind(&to_string<column_value_type>, boost::bind(r, boost::placeholders::_1), boost::placeholders::_2);
			else
				rc.m_text_renderer=boost::bind(f, boost::bind(r, boost::placeholders::_1), boost::placeholders::_2);
			rc.m_comparator=boost::bind(r, boost::placeholders::_1) < boost::bind(r, boost::placeholders::_2);
			return rc;
		}

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
					return add(make_basic_column_descriptor_memfnc<value_type>(title, width, ptr_mem_fnc, f));
				}
				template <typename T1>
				back_inserter operator()(const tstring &title, int width, const T1 &v, typename column_accessor<T1, value_type>::formatter_type f=0, typename boost::disable_if<boost::is_void<typename T1::result_type> >::type *dmy=0) const
				{
					return add(make_basic_column_descriptor<value_type>(title, width, v, f));
				}
				template <typename TextRenderer>
				back_inserter operator()(const tstring &title, int width, TextRenderer t, typename boost::enable_if<boost::is_void<typename TextRenderer::result_type> >::type* dmy=0) const
				{
					return add(column_descriptor_type(title, width, text_renderer_type(t)));
				}
            };
			back_inserter add;
			void set_columns(const std::initializer_list<column_descriptor_type> &cols)
			{
				set_dirty();
				m_column_data = cols;
			}
			void add_columns(const std::initializer_list<column_descriptor_type>& cols)
			{
				set_dirty();
				m_column_data.insert(m_column_data.end(), cols.begin(), cols.end());
			}
			void operator=(const std::initializer_list<column_descriptor_type>& cols)
			{
				set_columns(cols);
			}
			void operator+=(const std::initializer_list<column_descriptor_type>& cols)
			{
				add_columns(cols);
			}
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
				typename columns_t::const_iterator i=find_if(columns().begin(), columns().end(), boost::bind(&columns_t::value_type::title, boost::placeholders::_1)==title);
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
						typename columns_t::iterator i;
						i=std::find_if(dest.begin(), dest.end(), boost::bind(&columns_t::value_type::title, boost::placeholders::_1) == src.title());
						if (i!=dest.end()) {
							columns_t::value_type &current(*i);
							current.width(src.width());
							current.visible(src.visible());
							current.position(src.position());
							current.alignment(src.alignment());
						}
					}
				};
				for_each(in.begin(), in.end(), boost::bind(&merge_element::do_merge, boost::ref(m_column_data), boost::placeholders::_1));
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
                typename sorted_container_type::iterator dest=m_item_handles.begin();
                typename container_type::iterator i=container().begin();
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

            void get_item_text(size_t row, size_t col, tstring &rc)
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
				ui_adapter().for_each_selected(bind(&basic_list_mediator::visit, this, boost::placeholders::_1, f));
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
				ui_adapter().for_each_selected(bind(&basic_list_mediator::visit, this, &rc, boost::placeholders::_1, f));
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

BOOST_CLASS_VERSION(litwindow::ui::basic_column_label, 2);

#pragma optimize("", on)
#endif // list_mediator_h__31080910
