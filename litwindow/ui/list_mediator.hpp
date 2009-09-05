#ifndef list_mediator_h__31080910
#define list_mediator_h__31080910

#include "litwindow/tstring.hpp"

namespace litwindow {
    namespace ui {

        class basic_element_accessor
        {
        public:
        };

        class basic_column_descriptor
        {
        public:
            /// the title or header of the column
            tstring title() const;
            /// the internal name of the column
            tstring name() const;
        };

        template <typename Dataset>
        class basic_dataset_accessor
        {
        public:
            typedef Dataset value_type;
            typedef typename Dataset::value_type element_type;

            void sort();
            void filter();
        };

        template <typename Dataset>
        inline basic_dataset_accessor<Dataset> make_dataset_accessor(Dataset &d)
        {
            return basic_dataset_accessor<Dataset>();
        }

        template <typename List>
        class basic_list_adapter
        {
        public:
            typedef List value_type;
        };

        template <typename Dataset, typename List>
        class basic_list_mediator
        {
        public:
            typedef basic_list_adapter<List> list_adapter_type;
            typedef basic_dataset_accessor<Dataset> dataset_accessor_type;

            typedef typename list_adapter_type::value_type list_type;
            typedef typename dataset_accessor_type::value_type dataset_type;
            typedef typename dataset_type::value_type element_type;

            void set_ui(const list_adapter_type &a) {}
            template <typename List>
            void set_ui(List &l)
            {
                set_ui(make_list_adapter(l));
            }

            void set_list(const dataset_accessor_type &d) {}
            template <typename Data>
            void set_list(Data &d)
            {
                set_list(make_dataset_accessor(d));
            }
        };

    }
}
#endif // list_mediator_h__31080910