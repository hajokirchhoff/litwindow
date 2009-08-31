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

        class basic_dataset_accessor
        {
        public:
            void sort();
            void filter();
        };

        class basic_list_adapter
        {
        public:
        };

        class basic_list_mediator
        {
        public:
            void set_ui();
            void set_list();
        };

    }
}
#endif // list_mediator_h__31080910