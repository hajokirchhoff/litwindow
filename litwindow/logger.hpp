#ifndef litwindow_logger_h__151208
#define litwindow_logger_h__151208

#include <iostream>

namespace litwindow {
    namespace logger {

        template <typename _Elem>
        class basic_instance
        {
        public:
            basic_instance(){}
            basic_instance(const std::basic_string<_Elem> &name)
                :m_name(name)
                ,m_instance(next_instance_value())
            {
            }
            void open(const std::basic_string<_Elem> &name)
            {
                m_name=name;
                m_instance=next_instance_value();
            }
            void close()
            {
                m_name.clear();
            }
        private:
            static int next_instance_value()
            {
                static int g_instance=0;
                return g_instance++;
            }
            std::basic_string<_Elem> m_name;
            unsigned int m_instance;
        };

        template <typename _Elem>
        class category
        {
        public:
            std::basic_string<_Elem> value_name(int v) const { return boost::lexical_cast<std::basic_string<_Elem> >(v); }
            std::basic_string<_Elem> category_name() const { return std::basic_string<_Elem>(); }
        };

        namespace levels {
            enum default_level_enum
            {
                debug_level,
                info,
                warning,
                error,
                critical
            };
            template <typename _Elem>
            class default_level_category:public category<_Elem>
            {
                inline std::basic_string<_Elem> value_name(int v) const;
                inline std::basic_string<_Elem> category_name() const;
            };

            template<>
            inline std::basic_string<char> default_level_category<char>::value_name(int v) const
            {
                switch (v)
                {
                case debug_level: return "debug";
                case info: return "info";
                case warning: return "warning";
                case error: return "error";
                case critical: return "critical";
                }
                return "unknown";
            }
            template<>
            inline std::basic_string<char> default_level_category<char>::category_name() const { return "default"; }

            template<>
            inline std::basic_string<wchar_t> default_level_category<wchar_t>::value_name(int v) const
            {
                switch (v)
                {
                case debug_level: return L"debug";
                case info: return L"info";
                case warning: return L"warning";
                case error: return L"error";
                case critical: return L"critical";
                }
                return L"unknown";
            }
            template<>
            inline std::basic_string<wchar_t> default_level_category<wchar_t>::category_name() const { return L"default"; }
        }

        template <int Value, typename _Elem/*, typename Category=levels::default_level_category<_Elem>()*/>
        class basic_level
        {
        public:
            //typedef Category category_type;
            //basic_level(Category c=Category())
            //    :m_category(c)
            //{
            //}
            int get() const { return Value; }
        private:
            //Category m_category;
        };

        const basic_level<levels::critical, wchar_t> wcritical=basic_level<levels::critical, wchar_t>();
        const basic_level<levels::error, wchar_t> werror=basic_level<levels::error, wchar_t>();

        template <typename _Streambuf >
        class basic_log_streambuf:public _Streambuf
        {
            typedef typename _Streambuf::traits_type traits_type;
            typedef typename _Streambuf::int_type int_type;
            typedef typename _Streambuf::char_type char_type;
        public:
            basic_log_streambuf():end_of_log_entry(0){}
            void set_level(int lvl) {}
            virtual int sync()
            {
                //TODO: Write buffer to log stream
                return 0;
            }
            virtual int_type overflow(int_type _Meta = traits_type::eof() )
            {
                int_type rc;
                if (_Meta = end_of_log_entry ) {
                    pubsync();
                    rc = traits_type::not_eof(_Meta);
                } else {
                    rc = _Streambuf::overflow(_Meta);
                }
                return rc;
            }
            void set_end_of_log_entry(int_type new_end_of_Log_entry)
            {
                end_of_log_entry=new_end_of_Log_entry;
            }
        private:
            int_type end_of_log_entry;
        };

        template <typename _Elem, typename Streambuf=basic_log_streambuf<std::basic_streambuf<_Elem> >, typename _Outstream=std::basic_ostream<_Elem, std::char_traits<_Elem> > >
        class basic_events:public Streambuf
        {
            typedef basic_events<_Elem, Streambuf> _Myt;
            typedef _Outstream outstream_type;
            typedef std::basic_ostream<_Elem>& (*iomanipulator)(std::basic_ostream<_Elem>&);
            typedef _Myt& (*logmanipulator)(_Myt&);
            outstream_type _outstream;
            bool _enabled;
        public:
            class inserter
            {
            public:
                typedef _Myt events_type;
                bool _enabled;
                events_type &_owner;

                inserter(events_type &owner, bool is_enabled):_owner(owner),_enabled(is_enabled){}
                template <typename Value>
                inserter &operator &(const Value &v)
                {
                    if (_enabled) {
                        _owner.put(v);
                    }
                    return *this;
                }
                events_type &operator&(logmanipulator pFn)
                {
                    return (*pFn)(_owner);
                }
                inserter &operator&(iomanipulator pFn)
                {
                    if (pFn==std::endl) {

                    }
                    (*pFn)(_owner._outstream);
                    return *this;
                }
                inserter &operator<<(std::basic_ostream<_Elem> &(*pFn)(std::basic_ostream<_Elem> &))
                {
                    return operator&(pFn);
                }
                template <typename Value>
                inserter &operator <<(const Value &v)
                {
                    return operator&(v);
                }
            };


            basic_events():_outstream(rdbuf()),_enabled(true)
            {
            }
            basic_events(const std::basic_string<_Elem> &name):_outstream(rdbuf())
                ,m_instance(name),_enabled(true)
            {
            }
            void open(const std::basic_string<_Elem> &name) { m_instance.open(name); }
            void close() { m_instance.close(); }
            void enabled(bool is_enabled) { _enabled=is_enabled; }
            bool enabled() const { return _enabled; }

            template <typename Value>
            inserter &operator<<(const Value &v)
            {
                return inserter(*this, true) << v;
            }
            //_Myt &operator << (basic_ostream<_Elem> &(*fnc)(basic_ostream<_Elem>&)) 
            //{ 
            //    return *this;
            //}
            //_Myt &operator << (_Myt& (*m)(_Myt &))
            //{ 
            //    return (*m)(*this); 
            //}
            virtual void do_begin() {}      ///< begin a new entry
            virtual void do_end() {}        ///< end an entry

            template <typename Value>
            inline inserter &operator&(const Value &v)
            {
                return inserter(*this, _enabled) & v;
            }
            inline _Myt &operator&(logmanipulator pFn)
            {
                return inserter(*this, _enabled) & pFn;
            }
            inline _Myt &operator<<(logmanipulator pFn)
            {
                return operator&(pFn);
            }
            inline inserter &operator&(std::basic_ostream<_Elem> &(*pFn)(std::basic_ostream<_Elem> &_Ostr))
            {
                return inserter(*this, _enabled) & pFn;
            }
            inline inserter &operator<<(std::basic_ostream<_Elem> &(*pFn)(std::basic_ostream<_Elem> &_Ostr))
            {
                return inserter(*this, _enabled) & pFn;
            }
            template <typename Value>
            inline void put(const Value &v)
            {
                _outstream << v;
            }
        private:
            Streambuf *rdbuf() { return static_cast<Streambuf*>(this); }
            basic_instance<_Elem> m_instance;
        };


        template <typename _Elem, typename Streambuf>
        inline basic_events<_Elem, Streambuf> &information(basic_events<_Elem, Streambuf> &in) { return in; }

        template <typename _Elem, typename Streambuf>
        inline basic_events<_Elem, Streambuf> &warning(basic_events<_Elem, Streambuf> &in) { return in; }

        template <typename _Elem, typename Streambuf>
        inline basic_events<_Elem, Streambuf> &error(basic_events<_Elem, Streambuf> &in) { return in; }

        template <typename _Elem, typename Streambuf>
        inline basic_events<_Elem, Streambuf> &disable(basic_events<_Elem, Streambuf> &in) { in.enabled(false); return in; }
        template <typename _Elem, typename Streambuf>
        inline basic_events<_Elem, Streambuf> &enable(basic_events<_Elem, Streambuf> &in) { in.enabled(true); return in; }
        template <typename _Elem, typename Streambuf>
        inline basic_events<_Elem, Streambuf> &endl(basic_events<_Elem, Streambuf> &in) { in.put('\n'); return in; }

        typedef basic_events<char> events;
        typedef basic_events<wchar_t> wevents;

    }
}

#ifdef _MSC_VER
#pragma once
#endif

#endif // litwindow_logger_h__151208
