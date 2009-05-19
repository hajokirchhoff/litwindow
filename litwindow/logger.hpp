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


        //---------------------------------------------------------------------------------------------
        // 
        template <typename _Elem>
        class basic_logsink;
        template <typename _Elem>
        inline basic_logsink<_Elem> *global_sink();

        typedef basic_logsink<char> logsink;
        typedef basic_logsink<wchar_t> wlogsink;

        template <typename _Elem, typename _Traits, typename _Alloc >
        class basic_logbuf:public std::basic_streambuf<_Elem, _Traits>
        {
            friend class basic_logsink<_Elem>;
        public:
            enum {
                _Allocated = 1  ///< set if character array storage has been allocated
            };
            typedef int _Strstate;
            typedef typename _Traits::int_type int_type;
            typedef typename _Traits::pos_type pos_type;
            typedef typename _Traits::off_type off_type;
            typedef basic_streambuf<_Elem, _Traits> _Mysb;
            typedef _Alloc allocator_type;
            typedef basic_logsink<_Elem> sink_type;
            typedef time_t timestamp_type;
        private:
            struct entry
            {
                timestamp_type  m_ts;
                unsigned int    m_instance;
                unsigned int    m_source:16;
                unsigned int    m_topic:16;
                unsigned int    m_level:16;
                unsigned int    m_length:16;
            };
            enum {
                GROW_INCREMENT = 4096   ///< number of elements the log buffer is grown per overflow request
            };
        public:
            basic_logbuf():end_of_log_entry(0), m_sink(global_sink<_Elem>()) { init(); }
            ~basic_logbuf() { clear(); sink(0); }
            void sink(sink_type *new_sink) { m_sink=new_sink; }
            sink_type *sink() const     { return m_sink; }

            void    level(size_t lvl)   { m_level=lvl; }
            size_t  level() const       { return m_level; }
            void    topic(size_t t)     { m_topic=t; }
            size_t  topic() const       { return m_topic; }
            void    source(size_t s)    { m_source=s; }
            size_t  source() const      { return m_source; }
            virtual int sync()
            {
                int rc=_Mysb::sync();
                //TODO: Optimize this
                if (sink()) {
                    sink()->put(_Mysb::pbase(), _Mysb::pptr()-_Mysb::pbase());
                    _Mysb::setp(_Mysb::pbase(), _Mysb::pbase(), _Mysb::epptr());
                    m_begin_entry=0;
                }
                return rc;
            }

            virtual int_type overflow(int_type _Meta = traits_type::eof() )
            {
                if (traits_type::eq_int_type(traits_type::eof(), _Meta))
                    return traits_type::not_eof(_Meta);
                // if room in buffer, then store element
                if (_Mysb::pptr()!=0 && _Mysb::pptr() < _Mysb::epptr()) {
                    *_Mysb::pptr() = traits_type::to_char_type(_Meta);
                    _Mysb::pbump(1);
                    return _Meta;
                }
                // grow buffer
                size_t _Oldsize = _Mysb::pptr() == 0 ? 0 : _Mysb::epptr() - _Mysb::pbase();
                size_t _Newsize = _Oldsize + GROW_INCREMENT;
                _Elem *_Newptr = m_allocator.allocate(_Newsize);
                if (_Oldsize == 0) {
                    // first growth, set up buffer and pointers
                    _Mysb::setp(_Newptr, _Newptr+_Newsize);
                } else {
                    _Elem *_Oldptr = _Mysb::pbase();
#ifdef _MSC_VER
                    _Traits_helper::copy_s<_Traits>(_Newptr, _Newsize, _Oldptr, _Oldsize);
#else
                    std::copy(_Oldptr, _Oldptr+_Oldsize, _Newptr);
#endif
                    _Mysb::setp(_Newptr, _Newptr + (_Mysb::pptr()-_Oldptr), _Newptr+_Newsize);
                    if (m_state & _Allocated)
                        m_allocator.deallocate(_Oldptr, _Oldsize);
                }
                m_state |= _Allocated;
                *_Mysb::pptr() = traits_type::to_char_type(_Meta);
                _Mysb::pbump(1);
                return _Meta;
            }
            void set_end_of_log_entry(int_type new_end_of_Log_entry)
            {
                end_of_log_entry=new_end_of_Log_entry;
            }
            void begin_entry()
            {
                puttime(); putsource(); puttopic(); putlevel();                
                m_begin_entry=pptr();
            }
            void end_entry()
            {
                off_t total=count();
                do_put(total);
            }
            void clear()
            {
                if (m_state & _Allocated) {
                    m_allocator.deallocate(_Mysb::pbase(), _Mysb::epptr()-_Mysb::pbase());
                    m_state&=~_Allocated;
                }
                _Mysb::setp(0,0);
            }
            std::basic_string<_Elem> str() const
            {
                return _Mysb::pptr() ? std::basic_string<_Elem>(_Mysb::pbase(), _Mysb::pptr()) : std::basic_string<_Elem>();
            }
        private:
            void    init()
            {
                m_state=0;
                clear();
            }
            size_t m_level;
            size_t m_source;
            size_t m_topic;
            template <typename Value>
            void do_put(const Value &v)
            {
                sputn((const char_type*)&v, sizeof(v)/sizeof(char_type));
            }
            size_t count() const { return pptr()-m_begin_entry; }
            timestamp_type timestamp() const { return time_t(0); }
            void puttime()      { do_put(timestamp()); }
            void putsource()    { do_put(m_source); }
            void puttopic()     { do_put(m_topic); }
            void putlevel()     { do_put(m_level); }

            char_type *m_begin_entry;
            int_type end_of_log_entry;
            sink_type *m_sink;
            allocator_type m_allocator;
            _Strstate m_state;
        };

        template <typename _Elem, typename _Traits=std::char_traits<_Elem>, typename _Alloc=std::allocator<_Elem> >
        class basic_logstream:public std::basic_ostream<_Elem, _Traits>
        {
            typedef std::basic_ostream<_Elem, _Traits> inherited;
        public:
            typedef basic_logbuf<_Elem, _Traits, _Alloc> _Streambuf;
            typedef basic_logstream<_Elem, _Traits> _Myt;
            typedef typename _Streambuf::sink_type sink_type;
            typedef _Myt& (*logmanipulator)(_Myt&);
            basic_logstream()
                :inherited(&m_rdbuf)
            {
            }
            _Streambuf *rdbuf() { return &m_rdbuf; }
            void put_ts();
            void put_level(levels::default_level_enum l)
            {
            }
            void put_topic();
            void sink(sink_type *newsink) { rdbuf()->sink(newsink); }
            sink_type *sink() const { return rdbuf()->sink(); }

            _Myt &operator &&(logmanipulator l)
            {
                return (*l)(*this);
            }

            _Myt &begin_entry()
            {
                m_rdbuf.begin_entry(); return *this;
            }
            _Myt &end_entry()
            {
                m_rdbuf.end_entry(); return *this;
            }
        protected:
            _Streambuf m_rdbuf;
            
        };
        typedef basic_logstream<char> logstream;
        typedef basic_logstream<wchar_t> wlogstream;

        template <typename _Stream>
        struct stream_traits
        {
            typedef _Stream stream_type;
            void put_level(_Stream &stream, levels::default_level_enum l) { }
            void set_sink(_Stream &stream, basic_logsink<typename _Stream::char_type> *s) { }
            void lbegin(_Stream &stream) { }
            void lend(_Stream &stream) { }
        };
        template <typename _Elem>
        struct stream_traits<basic_logstream<_Elem> >
        {
            typedef basic_logstream<_Elem> stream_type;
            void put_level(stream_type &stream, levels::default_level_enum l) { stream.put_level(l); }
            void set_sink(stream_type &stream, typename stream_type::sink_type *s) { stream.sink(s); }
            void lbegin(stream_type &stream) { stream.begin_entry(); }
            void lend(stream_type &stream) { stream.end_entry(); }
        };
        // ---------------------------------------------------------------------------------------------
        
        /// Base class for logging events
        template <
            typename _Elem,
            typename _Outstream=basic_logstream<_Elem> ,
            typename _Streamtraits=logger::stream_traits<_Outstream>
        >
        class basic_events:public _Outstream
        {
            typedef basic_events<_Elem, _Outstream, _Streamtraits> _Myt;
            typedef _Outstream outstream_type;
            typedef _Streamtraits outstream_traits;
            typedef std::basic_ostream<_Elem>& (*iomanipulator)(std::basic_ostream<_Elem>&);
            typedef _Myt& (*logmanipulator)(_Myt&);
            bool m_enabled;
            _Streamtraits m_stream_traits;
        public:
            class inserter
            {
            public:
                typedef _Myt events_type;
                bool _enabled;
                events_type &_owner;

                inserter(events_type &owner, bool is_enabled)
                    :_owner(owner)
                    ,_enabled(is_enabled)
                {
                    _owner.do_begin();
                }
                ~inserter()
                {
                    _owner.do_end();
                }
                template <typename Value>
                inserter &operator &&(const Value &v)
                {
                    if (_enabled) {
                        _owner.put(v);
                    }
                    return *this;
                }
                events_type &operator&&(logmanipulator pFn)
                {
                    return (*pFn)(_owner);
                }
                inserter &operator&&(iomanipulator pFn)
                {
                    if (pFn==std::endl) {

                    }
                    //(*pFn)(_owner._outstream);
                    return *this;
                }
                inserter &operator<<(std::basic_ostream<_Elem> &(*pFn)(std::basic_ostream<_Elem> &))
                {
                    return operator&&(pFn);
                }
                template <typename Value>
                inserter &operator <<(const Value &v)
                {
                    return operator&&(v);
                }
            };


            basic_events():m_enabled(true)
            {
            }
            basic_events(const std::basic_string<_Elem> &name)
                :m_instance(name),m_enabled(true)
            {
            }
            basic_events(basic_logsink<_Elem> *target):m_enabled(true)
            {
                sink(target);
            }
            void open(const std::basic_string<_Elem> &name) { m_instance.open(name); }
            void close() { m_instance.close(); }
            void enabled(bool is_enabled) { m_enabled=is_enabled; }
            void enable() { enabled(true); }
            void disable() { enabled(false); }
            bool enabled() const { return m_enabled; }

            _Outstream &stream() { return *this; }

            operator bool() const { return m_enabled; }

            template <typename Value>
            inserter operator && (const Value &v)
            {
                return inserter(*this, true) && v;
            }
            template <typename Value>
            inserter operator<<(const Value &v)
            {
                return inserter(*this, true) << v;
            }
            void do_begin() { m_stream_traits.lbegin(*this); }      ///< begin a new entry
            void do_end() { m_stream_traits.lend(*this); }        ///< end an entry

            //const Streambuf *rdbuf() const { return static_cast<const Streambuf*>(this); }
            //Streambuf *rdbuf() { return static_cast<Streambuf*>(this); }

            inserter operator && (logmanipulator l)
            {
                _Myt &r((*l)(*this));
                return inserter(r, r.enabled());
            }
            void level(levels::default_level_enum l)
            {
                m_stream_traits.put_level(*this, l);
            }
            template <typename Value>
            void put(const Value &v)
            {
                stream() << v;
            }
            void sink(basic_logsink<_Elem> *s)
            {
                m_stream_traits.set_sink(stream(), s);
            }
        private:
            friend class inserter;
            basic_instance<_Elem> m_instance;
        };

        template <typename _Elem, typename Streambuf>
        inline basic_events<_Elem, Streambuf> &debug(basic_events<_Elem, Streambuf> &in) { in.level(levels::debug_level); return in; }
        template <typename _Elem, typename Streambuf>
        inline basic_events<_Elem, Streambuf> &information(basic_events<_Elem, Streambuf> &in) { in.level(levels::information); return in; }
        template <typename _Elem, typename Streambuf>
        inline basic_events<_Elem, Streambuf> &warning(basic_events<_Elem, Streambuf> &in) { in.level(levels::warning); return in; }
        template <typename _Elem, typename Streambuf>
        inline basic_events<_Elem, Streambuf> &error(basic_events<_Elem, Streambuf> &in) { in && basic_level<levels::error, _Elem>(); return in; }
        template <typename _Elem, typename Streambuf>
        inline basic_events<_Elem, Streambuf> &critical(basic_events<_Elem, Streambuf> &in) { in && basic_level<levels::critical, _Elem>(); return in; }

        template <typename _Elem, typename Streambuf>
        inline basic_events<_Elem, Streambuf> &disable(basic_events<_Elem, Streambuf> &in) { in.disable(); return in; }
        template <typename _Elem, typename Streambuf>
        inline basic_events<_Elem, Streambuf> &enable(basic_events<_Elem, Streambuf> &in) { in.enable(); return in; }
        //template <typename _Elem, typename Streambuf>
        //inline basic_events<_Elem, Streambuf> &endl(basic_events<_Elem, Streambuf> &in) { in.put('\n'); return in; }

        typedef basic_events<char> events;
        typedef basic_events<wchar_t> wevents;
        typedef events log;
        typedef wevents wlog;



        template <typename _Elem>
        class basic_logsink
        {
        public:
            void put(const _Elem *buffer, size_t count)
            {
                m_buffer+=basic_string<_Elem>(buffer, count);
            }
            void clear()
            {
                m_buffer.clear();
            }
            const std::basic_string<_Elem> &str() const { return m_buffer; }
        protected:
            std::basic_string<_Elem> m_buffer;
        };

        template <typename _Elem>
        inline basic_logsink<_Elem> *global_sink()
        {
            static basic_logsink<_Elem> g_sink;
            return &g_sink;
        }



    }
}

#ifdef _MSC_VER
#pragma once
#endif

#endif // litwindow_logger_h__151208
