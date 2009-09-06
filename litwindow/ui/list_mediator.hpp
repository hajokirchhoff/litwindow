#ifndef list_mediator_h__31080910
#define list_mediator_h__31080910

#include "litwindow/tstring.hpp"
#include <vector>
#include <boost/ref.hpp>

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
        public:
            basic_column_descriptor(const tstring &name, const tstring &title=tstring())
                :m_name(name), m_title(title.empty() ? name : title) {}
            /// the title or header of the column
            tstring title() const { return m_title; }
            /// the internal name of the column
            tstring name() const { return m_name; }
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

        typedef std::vector<basic_column_descriptor> basic_columns;

        template <typename List>
        class basic_ui_control_adapter
        {
        public:
            typedef List value_type;

            size_t column_count() const;
            size_t item_count() const;
        };

        template <typename DatasetAdapter, typename UIControlAdapter, typename ColumnsAdapter=basic_columns>
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
            
            void begin_update() {}
            void end_update() {}

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
            if (m_columns.size() != m_ui_control_adapter.column_count())
                do_refresh=true;
            if (do_refresh) {
                begin_update();
                size_t c=0;
                while (c<m_columns.size()) {
                    if (c>=m_ui_control_adapter.column_count())
                        m_ui_control_adapter.insert_column(c, m_columns[c]);
                    else
                        m_ui_control_adapter.set_column(c, m_columns[c]);
                }
                while (m_ui_control_adapter.column_count()>=m_columns.size())
                    m_ui_control_adapter.remove_column(m_ui_control_adapter.column_count()-1);
                end_update();
                m_needs_refresh_columns=false;
            }
        }

        template <typename DatasetAdapter, typename UIControlAdapter, typename ColumnsAdapter>
        void litwindow::ui::basic_list_mediator<DatasetAdapter, UIControlAdapter, ColumnsAdapter>::refresh_list()
        {

        }
    }
}
#endif // list_mediator_h__31080910