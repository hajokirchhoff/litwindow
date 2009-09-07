#ifndef list_mediator_h__31080910
#define list_mediator_h__31080910

#include "litwindow/tstring.hpp"
#include <vector>
#include <boost/ref.hpp>
#include <boost/function.hpp>
#include <iterator>

namespace litwindow {
    namespace ui {
    }
}


namespace litwindow {
    namespace ui {

        class basic_element_accessor
        {
        public:
        };

        class basic_column_descriptor
        {
            tstring m_title;
            tstring m_name;
            int     m_width;
            bool    m_visible;
        public:
            basic_column_descriptor(const tstring &name, const tstring &title=tstring(), int width=-1)
                :m_name(name), m_title(title.empty() ? name : title), m_width(width), m_visible(true)
            {}
            /// the title or header of the column
            tstring title() const { return m_title; }
            /// the internal name of the column
            tstring name() const { return m_name; }
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
                ar & m_title & m_name & m_width & m_visible;				
            }
        };

        template <typename ElementAdapter>
        class basic_columns_adapter
        {
        public:
            typedef ElementAdapter element_adapter;
            typedef typename element_adapter::element_value_type element_value_type;
            typedef typename element_adapter::value_type value_type;
            typedef typename element_adapter::column_position_type column_position_type;
            typedef basic_column_descriptor column_descriptor_type;

            typedef std::vector<column_descriptor_type> columns_t;
            const columns_t &columns() const { return m_column_data; }
            std::back_insert_iterator<columns_t> add(const basic_column_descriptor &d)
            {
                set_dirty();
                m_column_data.push_back(d);
                return std::back_inserter<columns_t>(m_column_data);
            }
            struct back_inserter
            {
                basic_columns_adapter &m_c;
                back_inserter(basic_columns_adapter &c):m_c(c){}
                back_inserter operator()(const basic_column_descriptor &d) const
                {
                    m_c.add(d);
                    return *this;
                }
                back_inserter operator()(const tstring &name, const tstring &title, int width=-1) const
                {
                    return operator()(basic_column_descriptor(name, title, width));
                }
            };
            back_inserter add(const tstring &name, const tstring &title, int width=-1)
            {
                return back_inserter(*this)(name, title, width);
            }
            size_t size() const { return columns().size(); }
            bool dirty() const { return m_dirty; }
            basic_columns_adapter()
                :m_dirty(true){}
            const value_type &column_description(size_t idx) const { return columns().at(idx); }

            const element_value_type &get(const value_type &e, const column_position_type &pos) const
            {
                return m_element_adapter(e, pos);
            }
        protected:
            void set_dirty() { m_dirty=true; }
            void clear_dirty() { m_dirty=false; }
        private:
            bool m_dirty;
            columns_t m_column_data;
            element_adapter m_element_adapter;
        };

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
            typedef typename columns_adapter_type::element_value_type element_value_type;
            typedef typename columns_adapter_type::column_position_type column_position_type;
            typedef boost::function<bool(const value_type *left, const value_type *right)> sort_pred_type;
            typedef boost::function<bool(const value_type *f)> filter_pred_type;
        private:
            container_type *m_c;
            sorted_container_type m_ptrs;
            columns_adapter_type m_columns_adapter;
            sort_pred_type m_sort_pred;
            filter_pred_type m_filter_pred;

            container_type &container() const { return *m_c; }
        public:
            _Myt &set_container(container_type &v)
            {
                m_c=&v;
                update();
                return *this;
            }
            _Myt &set_columns(const columns_adapter_type &cadapter)
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
                    if (m_filter_pred && m_filter_pred(new_v))
                        *dest++=new_v;
                    ++i;
                }
                while (i!=container().end()) {
                    sorted_container_type::value_type new_v= & *i;
                    if (m_filter_pred && m_filter_pred(new_v))
                        m_ptrs.push_back(new_v);
                    ++i;
                }
                m_ptrs.erase(dest, m_ptrs.end());
                if (m_sort_pred)
                    std::sort(m_ptrs.begin(), m_ptrs.end(), m_sort_pred);
            }
            void update() { sort(); }
            const value_type &value_at(size_t pos) const { return *m_ptrs.at(pos); }
            struct column_adapter
            {
                columns_adapter_type &cat;
                const value_type &value;
                const element_value_type &at(const column_position_type &p) const
                {
                    return cat.get(value, p);
                }
                column_adapter(columns_adapter_type &c, const value_type &v)
                    :cat(c),value(v){}
            };
            column_adapter at(size_t pos) const { return column_adapter(m_columns_adapter, at(pos)); }
            element_value_type at(size_t pos, const column_position_type &p) const { return at(pos).at(p); }
        };

        template <typename DatasetAdapter>
        inline basic_dataset_accessor<DatasetAdapter> make_dataset_accessor(DatasetAdapter &d)
        {
            return basic_dataset_accessor<DatasetAdapter>(d);
        }

        template <typename ColumnTraits, typename UIControl, typename Columns>
        void setup_columns(ColumnTraits &tr, UIControl c, Columns &cols)
        {
            size_t idx=0;
            while (idx<cols.size()) {
                if (idx>=tr.count(c))
                    tr.insert_column(c, idx, cols.at(idx));
                else
                    tr.set_column(c, idx, cols.at(idx));
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
        };

//////////////////////////////////////////////////////////////////////////
        template <typename DatasetAdapter, typename UIControlAdapter, typename ColumnsAdapter=basic_columns_adapter>
        class basic_list_mediator
        {
        public:

            void refresh();

            typedef UIControlAdapter ui_control_adapter_type;
            typedef DatasetAdapter dataset_accessor_type;

            typedef typename dataset_accessor_type::value_type element_type;
            typedef typename ColumnsAdapter columns_adapter_type;

            void set_columns(const columns_adapter_type &cols) { m_columns=cols; m_needs_refresh_columns=true; }
            columns_adapter_type &columns() { return m_columns; }
            const columns_adapter_type &columns() const { return m_columns; }

            template <typename List>
            void set_ui(List &l)
            {
                set_ui_adapter(make_list_adapter(l));
            }
            void set_ui_adapter(const ui_control_adapter_type &a) { m_ui_control_adapter=a; }

            void set_dataset_adapter(const dataset_accessor_type &d) { m_dataset_adapter=d; }
            template <typename Data>
            void set_dataset(Data &d)
            {
                set_dataset_adapter(make_dataset_accessor(d));
            }

            basic_list_mediator()
                :m_needs_refresh_columns(true){}
        protected:
            void refresh_columns(bool do_refresh=true);
            void refresh_list();
            
            void begin_update() { m_ui_control_adapter.begin_update(); }
            void end_update() { m_ui_control_adapter.end_update(); }

            void setup_columns();

            ui_control_adapter_type m_ui_control_adapter;
            columns_adapter_type    m_columns;
            dataset_accessor_type   m_dataset_adapter;
            bool            m_needs_refresh_columns;
        };


        //////////////////////////////////////////////////////////////////////////
        template <typename DatasetAdapter, typename UIControlAdapter, typename ColumnsAdapter>
        void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter, ColumnsAdapter>::refresh()
        {
            begin_update();
            refresh_columns(m_needs_refresh_columns);
            refresh_list();
            end_update();
        }

        template <typename DatasetAdapter, typename UIControlAdapter, typename ColumnsAdapter>
        void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter, ColumnsAdapter>::refresh_columns(bool do_refresh)
        {
            if (do_refresh || m_columns.dirty()) {
                begin_update();
                setup_columns();
                end_update();
                m_needs_refresh_columns=false;
            }
        }

        template <typename DatasetAdapter, typename UIControlAdapter, typename ColumnsAdapter>
        void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter, ColumnsAdapter>::refresh_list()
        {
            m_ui_control_adapter.set_item_count(m_dataset_adapter.size());
        }

        template <typename DatasetAdapter, typename UIControlAdapter, typename ColumnsAdapter>
        void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter, ColumnsAdapter>::setup_columns()
        {
            m_ui_control_adapter.setup_columns(m_columns);
        }

    }
}
#endif // list_mediator_h__31080910

void setup_columns();