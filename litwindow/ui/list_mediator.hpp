#ifndef list_mediator_h__31080910
#define list_mediator_h__31080910

#include "litwindow/tstring.hpp"
#include <vector>
#include <boost/ref.hpp>
#include <iterator>

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

        class basic_columns_adapter
        {
        public:
            typedef basic_column_descriptor value_type;
            typedef std::vector<value_type> columns_t;
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
            const value_type &at(size_t idx) const { return columns().at(idx); }
        protected:
            void set_dirty() { m_dirty=true; }
            void clear_dirty() { m_dirty=false; }
        private:
            bool m_dirty;
            columns_t m_column_data;
        };

        template <typename DatasetAdapter>
        class basic_dataset_accessor
        {
        public:
            typedef DatasetAdapter value_type;
            typedef typename DatasetAdapter::value_type element_type;

            void sort();
            void filter();
        };

        template <typename DatasetAdapter>
        inline basic_dataset_accessor<DatasetAdapter> make_dataset_accessor(DatasetAdapter &d)
        {
            return basic_dataset_accessor<DatasetAdapter>();
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

            void set_dataset_adapter(const dataset_accessor_type &d) {}
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
            bool            m_needs_refresh_columns;
        };


        //////////////////////////////////////////////////////////////////////////
        template <typename DatasetAdapter, typename UIControlAdapter, typename ColumnsAdapter>
        void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter, ColumnsAdapter>::refresh()
        {
            refresh_columns(m_needs_refresh_columns);
            refresh_list();
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