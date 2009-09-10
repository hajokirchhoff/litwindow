#ifndef list_mediator_h__31080910
#define list_mediator_h__31080910

#include "litwindow/tstring.hpp"
#include <vector>
#include <boost/ref.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <iterator>

namespace litwindow {
    namespace ui {
        template <typename Value/*, typename TextRenderer*/>
        class basic_column_descriptor
        {
        public:
            typedef boost::function<void (const Value&, tstring&)> text_renderer_type;
            //typedef TextRenderer text_renderer_type;
            typedef Value value_type;
        private:
            tstring m_title;
            int     m_width;
            bool    m_visible;
            text_renderer_type m_text_renderer;
        public:
            basic_column_descriptor(const tstring &title, int width=-1, const text_renderer_type &v=text_renderer_type())
                :m_title(title), m_width(width), m_visible(true), m_text_renderer(v)
            {}
            /// the title or header of the column
            tstring title() const { return m_title; }
            /// set the column width
            void width(int new_width) { m_width=new_width; }
            /// return the column width
            int width() const { return m_width; }
            /// visibility
            bool visible() const { return m_visible; }
            void visible(bool do_show) { m_visible=do_show; }
            template <typename Archive>
            void serialize(Archive &ar, const unsigned int version)
            {
                ar & m_title & m_width & m_visible;				
            }

            template <typename Control>
            bool render_element(size_t row, size_t col, Control &c, const value_type &v) const { return false; }

            bool render_element(tstring &c, const value_type &v) const
            {
                m_text_renderer(v, c);
                return true; 
            }

            //tstring value_as_text(const typename text_renderer_type::value_type &t) const;// { return m_text_renderer(t); }
        };

        template <typename Value>
        inline void to_string(const Value &v, tstring &c)
        {
            c=boost::lexical_cast<tstring>(v);
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

        template <typename ColumnDescriptor>
        class basic_columns_adapter
        {
        public:
            //typedef Value element_adapter;
            //typedef typename element_adapter::element_value_type element_value_type;
            //typedef typename element_adapter::column_position_type column_position_type;
            typedef ColumnDescriptor column_descriptor_type;
            typedef typename column_descriptor_type::value_type value_type;
            typedef typename column_descriptor_type::text_renderer_type text_renderer_type;

            typedef std::vector<column_descriptor_type> columns_t;
            const columns_t &columns() const { return m_column_data; }
            std::back_insert_iterator<columns_t> add(const column_descriptor_type &d)
            {
                set_dirty();
                m_column_data.push_back(d);
                return std::back_inserter<columns_t>(m_column_data);
            }
            struct back_inserter
            {
                basic_columns_adapter &m_c;
                back_inserter(basic_columns_adapter &c):m_c(c){}
                back_inserter operator()(const column_descriptor_type &d) const
                {
                    m_c.add(d);
                    return *this;
                }
                back_inserter operator()(const tstring &title, int width=-1, text_renderer_type r=text_renderer_type()) const
                {
                    return operator()(column_descriptor_type(title, width, r));
                }
            };
            template <typename ValueMember>
            back_inserter add(const tstring &title, int width=-1, ValueMember (value_type::*ptr_to_member)=0)
            {
                return back_inserter(*this)(title, width, make_text_renderer<ValueMember>(ptr_to_member));
            }
            template <typename ValueRenderer>
            back_inserter add(const tstring &title, int width=-1, ValueRenderer r=ValueRenderer())
            {
                return back_inserter(*this)(title, width, /*make_text_renderer<value_type>(r)*/r);
            }
            size_t size() const { return columns().size(); }
            bool dirty() const { return m_dirty; }
            basic_columns_adapter()
                :m_dirty(true){}
            const column_descriptor_type &column_descriptor(size_t idx) const { return columns().at(idx); }

            template <typename Control>
            bool render_element_at(size_t row, size_t col, Control &c, const value_type &v) const
            {
                return columns().at(col).render_element(row, col, c, v);
            }
            bool render_element_at(size_t col, tstring &rc, const value_type &v) const
            {
                return columns().at(col).render_element(rc, v);
            }
            typename columns_t::value_type &at(size_t col) { return columns().at(col); }
            //const value_type &column_description(size_t idx) const { return columns().at(idx); }

            //const element_value_type &get(const value_type &e, const column_position_type &pos) const
            //{
            //    return m_element_adapter(e, pos);
            //}
        protected:
            void set_dirty() { m_dirty=true; }
            void clear_dirty() { m_dirty=false; }
        private:
            bool m_dirty;
            columns_t m_column_data;
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

        template <typename Container, typename ColumnsAdapter>
        class stl_container_dataset_accessor
        {
        public:
            typedef stl_container_dataset_accessor<Container, ColumnsAdapter> _Myt;
            typedef Container container_type;
            typedef typename Container::value_type value_type;
            typedef std::vector<const value_type*> sorted_container_type;
            typedef ColumnsAdapter columns_adapter_type;
            //typedef typename columns_adapter_type::element_value_type element_value_type;
            //typedef typename columns_adapter_type::column_position_type column_position_type;
            typedef boost::function<bool(const value_type *left, const value_type *right)> sort_pred_type;
            typedef boost::function<bool(const value_type *f)> filter_pred_type;
        private:
            container_type *m_c;
            sorted_container_type m_ptrs;
            columns_adapter_type m_columns_adapter;
            sort_pred_type m_sort_pred;
            filter_pred_type m_filter_pred;
            bool m_needs_update;

            container_type &container() const { return *m_c; }
        public:
            const columns_adapter_type &columns_adapter() const { return m_columns_adapter; }
            columns_adapter_type &columns_adapter() { return m_columns_adapter; }
            _Myt &set_container(container_type &v)
            {
                m_c=&v;
                m_needs_update=true;
                return *this;
            }
            _Myt &set_columns_adapter(const columns_adapter_type &cadapter)
            {
                m_columns_adapter=cadapter;
                return *this;
            }
            size_t size() const { return m_ptrs.size(); }
            void sort()
            {
                sorted_container_type::iterator dest=m_ptrs.begin();
                container_type::const_iterator i=container().begin();
                while (dest!=m_ptrs.end() && i!=container().end()) {
                    sorted_container_type::value_type new_v= & *i;
                    if (!m_filter_pred || m_filter_pred(new_v))
                        *dest++=new_v;
                    ++i;
                }
                if (dest<m_ptrs.end())
                    m_ptrs.erase(dest, m_ptrs.end());
                else while (i!=container().end()) {
                    sorted_container_type::value_type new_v= & *i;
                    if (!m_filter_pred || m_filter_pred(new_v))
                        m_ptrs.push_back(new_v);
                    ++i;
                }
                if (m_sort_pred)
                    std::sort(m_ptrs.begin(), m_ptrs.end(), m_sort_pred);
            }
            void update() { if (m_needs_update) sort(); m_needs_update=false; }
            void refresh() { update(); }
            const value_type &value_at(size_t pos) const { return *m_ptrs.at(pos); }
        };


        template <typename Container, typename ColumnsAdapter>
        inline stl_container_dataset_accessor<Container, ColumnsAdapter> make_dataset_adapter(Container &d, const ColumnsAdapter &c)
        {
            stl_container_dataset_accessor<Container, ColumnsAdapter> rc;
            rc.set_container(d);
            rc.set_columns_adapter(c);
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
        };

//////////////////////////////////////////////////////////////////////////
        template <typename DatasetAdapter, typename UIControlAdapter>
        class basic_list_mediator
        {
        public:
            void refresh();

            typedef UIControlAdapter ui_control_adapter_type;
            typedef DatasetAdapter dataset_adapter_type;
            typedef typename dataset_adapter_type::columns_adapter_type columns_adapter_type;

            //void set_columns_adapter(const columns_adapter_type &cols) { m_columns=cols; m_needs_refresh_columns=true; }
            columns_adapter_type &columns_adapter() { return m_dataset_adapter.columns_adapter(); }
            const columns_adapter_type &columns_adapter() const { return m_dataset_adapter.columns_adapter(); }

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
            template <typename Data, typename Columns>
            void set_dataset(Data &d, const Columns &c)
            {
                set_dataset_adapter(make_dataset_adapter(d, c));
            }
            const dataset_adapter_type &dataset() const { return m_dataset_adapter; }

            basic_list_mediator()
                :m_needs_refresh_columns(true){}

            void get_item_text(size_t row, size_t col, tstring &rc) const
            {
                const dataset_adapter_type::columns_adapter_type &columns_adapter(dataset().columns_adapter());
                columns_adapter.render_element_at(col, rc, dataset().value_at(row));
            }

            void refresh_dataset()
            {
                m_dataset_adapter.refresh();
            }
        protected:
            void refresh_columns(bool do_refresh=true);
            void refresh_list();
            
            void begin_update() { m_ui_control_adapter.begin_update(); }
            void end_update() { m_ui_control_adapter.end_update(); }

            void setup_columns();

            ui_control_adapter_type m_ui_control_adapter;
            //columns_adapter_type    m_columns;
            dataset_adapter_type   m_dataset_adapter;
            bool            m_needs_refresh_columns;
        };


        //////////////////////////////////////////////////////////////////////////
        template <typename DatasetAdapter, typename UIControlAdapter>
        void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter>::refresh()
        {
            begin_update();
            refresh_dataset();
            refresh_columns(m_needs_refresh_columns);
            refresh_list();
            end_update();
        }

        template <typename DatasetAdapter, typename UIControlAdapter>
        void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter>::refresh_columns(bool do_refresh)
        {
            if (do_refresh || columns_adapter().dirty()) {
                begin_update();
                setup_columns();
                end_update();
                m_needs_refresh_columns=false;
            }
        }

        template <typename DatasetAdapter, typename UIControlAdapter>
        void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter>::refresh_list()
        {
            m_ui_control_adapter.refresh_list(m_dataset_adapter);
        }

        template <typename DatasetAdapter, typename UIControlAdapter>
        void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter>::setup_columns()
        {
            m_ui_control_adapter.setup_columns(columns_adapter());
        }

    }
}
#endif // list_mediator_h__31080910

void setup_columns();