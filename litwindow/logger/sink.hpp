#ifndef litwindow_logger_sink_h__151208
#define litwindow_logger_sink_h__151208

namespace litwindow {
	/// encapsules the litwindow logging library
    namespace logger {

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
			typedef std::list<logsink_type*> sink_chain_type;
			typedef typename logbuf_type::entries entries;
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
			void add_sink(logsink_type *s)
			{
				mutex_lock_type l(m_lock);
				m_sink_chain.push_back(s);
			}
			void remove_sink(logsink_type *s)
			{
				mutex_lock_type l(m_lock);
				m_sink_chain.remove(s);
			}
        protected:
			virtual void do_put(const entries &e) {}
			mutex_type					m_lock;
			sink_chain_type				m_sink_chain;
        };

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
			basic_logsink<_Elem> *g_the_sink;
			global_sink_data(basic_logsink<_Elem> *i=0) { g_the_sink=i; }
		};

        template <typename _Elem>
        inline basic_logsink<_Elem> *default_sink()
        {
			return global_sink_data<_Elem>::instance().get();
        }
		template <typename _Elem>
		inline void default_sink(basic_logsink<_Elem> *new_default_sink)
		{
			global_sink_data<_Elem>::instance().set(new_default_sink);
		}

		template <typename _OStream>
		class basic_ostream_logsink:public basic_logsink<typename _OStream::char_type>
		{
		public:
			typedef _OStream stream_type;
			typedef typename _OStream::char_type char_type;
			basic_ostream_logsink(stream_type &o)
				:m_out(&o)
				,m_sep(details::sep<char_type>())
			{
			}
			void sep(char_type new_separator) { m_sep=new_separator; }
			char_type sep() const { return m_sep; }
		protected:
			stream_type *m_out;
			char_type	m_sep;
			virtual void do_put(const entries &e)
			{
				entries::const_iterator i;
				for (i=e.begin(); i!=e.end(); ++i) {
					(*m_out) << i->component().str() << sep() << i->topic().str() << sep() << i->str() << endl;
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
