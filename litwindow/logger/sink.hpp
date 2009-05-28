#ifndef litwindow_logger_sink_h__151208
#define litwindow_logger_sink_h__151208

namespace litwindow {
	/// encapsules the litwindow logging library
    namespace logger {
        using boost::shared_ptr;

		//------------------------------------------------------------------------
		// 
        template <typename _Elem>
        class basic_logsink
        {
        public:
			typedef basic_logbuf<_Elem, std::char_traits<_Elem>, std::allocator<_Elem> > logbuf_type;
			typedef typename details::defaults<_Elem>::mutex_type mutex_type;
			typedef typename details::defaults<_Elem>::mutex_lock_type<mutex_type> mutex_lock_type;
			typedef basic_logsink<_Elem> logsink_type;
			typedef typename logbuf_type::entries entries;
			typedef typename entries::entry entry;
            typedef shared_ptr<logsink_type> logsink_type_ptr;
            typedef std::list<logsink_type_ptr> sink_chain_type;

            virtual ~basic_logsink()
            {
            }
            void put(const entries &e)
            {
				mutex_lock_type l(m_lock);
				do_put(e);
				for (sink_chain_type::const_iterator i=m_sink_chain.begin(); i!=m_sink_chain.end(); ++i)
					(*i)->do_put(e);
            }
            void clear()
            {
            }
			void add_sink(logsink_type_ptr s)
			{
				mutex_lock_type l(m_lock);
				m_sink_chain.push_back(s);
			}
			void remove_sink(const logsink_type_ptr &s)
			{
				mutex_lock_type l(m_lock);
				m_sink_chain.remove(s);
			}
        protected:
			virtual void do_put(const entries &e) {}
			mutex_type					m_lock;
			sink_chain_type				m_sink_chain;
        };


        typedef basic_logsink<char> sink;
        typedef basic_logsink<wchar_t> wsink;
		template <typename _Elem>
		struct global_sink_data
		{
			void set(basic_logsink<_Elem> *new_sink) { g_the_sink=new_sink; }
			basic_logsink<_Elem> *get() const { return g_the_sink; }
			static global_sink_data<_Elem> &instance()
			{
				static basic_logsink<_Elem> g_global_log_sink;
				static global_sink_data<_Elem> g_the_instance(&g_global_log_sink);
				return g_the_instance;
			}
		protected:
            ~global_sink_data()
            {
            }
			basic_logsink<_Elem> *g_the_sink;
			global_sink_data(basic_logsink<_Elem> *i=0) { g_the_sink=i; }
		};

        ///\return a pointer to the global default sink
        template <typename _Elem>
        inline basic_logsink<_Elem> *default_sink()
        {
			return global_sink_data<_Elem>::instance().get();
        }

        ///Set a new default sink. All logging instances created _after_ a call
        ///to this method will use the new sink as their default sink.
        ///This will not change the sink of existing logging instances.
		template <typename _Elem>
		inline void default_sink(basic_logsink<_Elem> *new_default_sink)
		{
			global_sink_data<_Elem>::instance().set(new_default_sink);
		}
		template <typename _Stream, typename _Entry>
		struct entry_formatter
		{
			bool timestamp, level, component, topic;
			typename _Stream::char_type sep;
			entry_formatter()
				:timestamp(true),level(true),component(true),topic(true),sep(details::sep<_Stream::char_type>())
			{}
			void operator()(_Stream &o, const _Entry &e) const
			{
				if (timestamp)
					o << e.timestamp() << sep;
				if (level)
					o << e.level() << sep;
				if (component)
					o << e.component() << sep;
				if (topic)
					o << e.topic() << sep;
				o << e.str() << endl;
			}
		};

		template <typename _OStream, typename _Formatter=entry_formatter<_OStream, typename basic_logsink<typename _OStream::char_type>::entry> >
		class basic_ostream_logsink:public basic_logsink<typename _OStream::char_type>
		{
		public:
			typedef _OStream stream_type;
			typedef typename _OStream::char_type char_type;
			typedef _Formatter formatter_type;
			basic_ostream_logsink(stream_type &o)
				:m_out(&o)
			{
			}
			void format(const formatter_type &e) { m_formatter=e; }
			formatter_type &format() { return m_formatter; }
		protected:
			formatter_type m_formatter;
			stream_type *m_out;
			virtual void do_put(const entries &e)
			{
				entries::const_iterator i;
				for (i=e.begin(); i!=e.end(); ++i) {
					m_formatter(*m_out, *i);
				}
			}
		};
		typedef basic_ostream_logsink<std::basic_ostream<char> > ostream_logsink;
		typedef basic_ostream_logsink<std::basic_ostream<wchar_t> > wostream_logsink;

#ifdef not
		template <typename _Elem>
		class basic_memory_logsink:public basic_logsink<_Elem>
		{
		public:
			typedef typename basic_logsink<_Elem>::entries entries;
			typedef std::list<boost::shared_ptr<entries> > entry_list_type;
			typedef basic_memory_logsink<_Elem> logsink_type;
			std::basic_string<_Elem> str() const { return std::basic_string<_Elem>(); }

			class iterator
			{
				friend class logsink_type;
				typename entries::const_iterator m_i;
				typename entry_list_type::const_iterator m_entry_iterator;
			public:
				iterator(){}
				iterator(typename entry_list_type::const_iterator &i):m_entry_iterator(i),m_i(i->begin()){}
				bool operator==(const iterator &i) const { return m_entry_iterator==i.m_entry_iterator && m_i==i.m_i; }
				bool operator!=(const iterator &i) const { return !operator==(i); }
				const iterator &operator++()
				{
					++m_i;
					if (m_i==(*m_entry_iterator)->end()) {
						++m_entry_iterator;
					}
				}
				const typename entries::const_iterator *operator->() const
				{
					return **m_i;
				}
			};
			iterator begin() const { return m_entries.size()>0 ? iterator(m_entries.begin()) : end(); }
			iterator end() const { return iterator(); }
		protected:
			void do_put(const entries &e)
			{
				m_entries.push_back(entry_list_type::value_type(new entries(e)));
			}
			entry_list_type m_entries;
		};

		typedef basic_memory_logsink<char> memory_logsink;
		typedef basic_memory_logsink<wchar_t> wmemory_logsink;
#endif // not

    }
}

#ifdef _MSC_VER
#pragma once
#endif

#endif // litwindow_logger_h__151208
