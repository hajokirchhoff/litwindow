#ifndef litwindow_logger_sink_h__151208
#define litwindow_logger_sink_h__151208

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <string>
#include <deque>
#include "../logger.hpp"

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
			mutable mutex_type			m_lock;
			sink_chain_type				m_sink_chain;
        };


        typedef basic_logsink<char> sink;
        typedef basic_logsink<wchar_t> wsink;

		template <typename _Elem>
		struct global_sink_data
		{
			void set(basic_logsink<_Elem> *new_sink) { g_the_sink=new_sink; }
			basic_logsink<_Elem> *get() const { return g_the_sink; }
			static global_sink_data<_Elem> LITWINDOW_LOGGER_API &instance()
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

		template <typename _Elem, size_t _Pagesize=64U*1024U>
		class basic_memory_logsink:public basic_logsink<_Elem>
		{
            static const size_t page_size=_Pagesize;
		public:
			typedef typename basic_logsink<_Elem>::entries entries;
            typedef typename entries::entry entry;
            basic_memory_logsink():m_page_count(0),m_first_entry_index(0),m_next_entry_index(0)
            {
            }
			~basic_memory_logsink()
			{

			}
        protected:
            struct page;
            typedef shared_ptr<page> page_ptr;
            struct page
            {
				size_t	m_page_number;
				size_t	m_first_index_in_page;
				basic_memory_logsink<_Elem, _Pagesize> *m_owner;
                unsigned char m_page[_Pagesize];
                const unsigned char *end_ptr() const { return m_page+_Pagesize; }
                unsigned char *end_ptr() { return m_page+_Pagesize; }
				const unsigned char *begin_ptr() const { return m_page; }
				unsigned char *begin_ptr() { return m_page; }
                size_t  available() const
                {
                    return end_ptr()-m_next;
                }
                page(basic_memory_logsink<_Elem, _Pagesize> *owner, size_t number, size_t first_index)
                    :m_next(begin_ptr())
					,m_owner(owner)
					,m_page_number(number)
					,m_first_index_in_page(first_index)
                {
                }
                unsigned char *m_next;
				const unsigned char *next_ptr() const { return m_next; }
                unsigned char *increment(unsigned char *from, size_t offset)
                {
                    // correct alignment if neccessary
                    size_t aligned_on=sizeof(unsigned long);
                    size_t mismatch=offset % aligned_on;
                    if (mismatch)
                        offset+=aligned_on-mismatch;
                    return from+offset;
                }
                bool    put(const entry &e, size_t index)
                {
					unsigned char *new_next=increment(m_next, e.full_size_in_bytes());
                    if (new_next>end_ptr()) {
						//TODO: split entry in two if it is longer than the available room
						// presently it is truncated.
                        if (end_ptr()-m_next<sizeof(entry)) {
                            // Error, not enough room
                            return false;
                        }
                        entry &copy_e(*reinterpret_cast<entry*>(m_next));
                        copy_e=e;
						copy_e.index(index);
                        _Elem *n=reinterpret_cast<_Elem*>(&copy_e+1);
                        const _Elem *f=e.begin_data();
                        while (reinterpret_cast<const unsigned char*>(n)+sizeof(_Elem)<=end_ptr()) {
                            *n++=*f++;
                        }
                        copy_e.m_length=unsigned short(f-e.begin_data());
                        m_next=end_ptr();
                    } else {
                        memcpy(m_next, &e, e.full_size_in_bytes());
						reinterpret_cast<entry*>(m_next)->index(index);
                        m_next=new_next;
                    }
                    return true;
                }
                void close()
                {
                    //entry close_e;
                    //close_e.m_timestamp=0;
                    //close_e.m_length=0;
                    //put(close_e);
                }
				typename entries::const_iterator begin() const { return entries((const _Elem*)begin_ptr(), (const _Elem*)next_ptr()).begin(); }
				typename entries::const_iterator end() const { return entries((const _Elem*)begin_ptr(), (const _Elem*)next_ptr()).end(); }
			};
			typedef std::deque<page_ptr> page_list_t;
			page_list_t m_pages;
			page_ptr	m_current;
			size_t	m_first_page_number;
			size_t	m_page_count;
			size_t	m_next_entry_index;
			size_t	m_first_entry_index;
			typedef basic_memory_logsink<_Elem> logsink_type;

		public:
			class const_iterator
			{
				friend class logsink_type;
				page_ptr m_page;
				typename entries::const_iterator m_i;
			public:
				const_iterator(){}
				const_iterator(page_ptr p):m_page(p),m_i(p->begin()){}
				bool operator==(const const_iterator &i) const { return m_page==i.m_page && m_i==i.m_i; }
				bool operator!=(const const_iterator &i) const { return !operator==(i); }
				const const_iterator &operator++()
				{
					++m_i;
					if (m_i==m_page->end()) {
						m_page=m_page->m_owner->next_page(m_page);
						if (m_page)
							m_i=m_page->begin();
						else
							m_i=entries::const_iterator();
					}
					return *this;
				}
				const entry *operator->() const
				{
					return &(*m_i);
				}
				const entry &operator*() const
				{
					return *m_i;
				}
			};
			const_iterator begin() const { return const_iterator(get_page(m_first_page_number)); }
			const_iterator end() const { return const_iterator(); }
			const_iterator find(size_t idx) const
			{
				mutex_lock_type l(m_lock);
				typename page_list_t::const_iterator page_i;
				page_i=do_find_page(idx);

				const_iterator rc(*page_i);
				while (rc->index()<idx && rc!=end()) {
					++rc;
				}
				return rc;
			}

			size_t	size() const { return m_next_entry_index-m_first_entry_index; }
			size_t	last_index() const { return m_next_entry_index-1; }
			size_t	first_index() const { return m_first_entry_index; }
			size_t	next_index() const { return m_next_entry_index; }
		protected:
			typename page_list_t::const_iterator do_find_page(size_t idx) const
			{
				if (m_pages.empty() || idx-m_first_entry_index>=m_next_entry_index)
					return m_pages.end();	// no such index
				typename page_list_t::const_iterator page_i;
				typename page_list_t::const_iterator rc;
				for (page_i=m_pages.begin(); page_i!=m_pages.end() && (*page_i)->m_first_index_in_page<=idx; ++page_i)
					rc=page_i;
				return rc;
			}
			page_ptr	get_page(size_t number) const
			{
				mutex_lock_type l(m_lock);
				number-=m_first_page_number;
				return number<m_pages.size() ? m_pages[number] : page_ptr();
			}
			page_ptr next_page(const page_ptr &p) const
			{
				size_t next_number=p->m_page_number;
				return get_page(next_number+1);
			}
            void alloc_new_page()
            {
				//The object already ownes the mutex at this point
				//mutex_lock_type l(m_lock);
                if (m_pages.empty()) {
					m_first_page_number=m_page_count;
                }
				m_current.reset(new page(this, m_page_count, m_next_entry_index));
				++m_page_count;
				m_pages.push_back(m_current);
            }
            void drop_from_begin(size_t page_count)
            {
                while (page_count-- && m_pages.size()>0) {
                    // shared_ptr will free the pages when they
                    // are no longer in use
					++m_first_page_number;
					m_pages.pop_front();
				}
				m_first_entry_index=m_pages.empty() ? m_next_entry_index : m_pages.begin()->m_first_index_in_page;
            }
            void drop_first_page()
            {
                drop_from_begin(1);
            }
            void put_entry(const entry &e)
            {
                if (m_current->available()<e.full_size_in_bytes()) {
                    alloc_new_page();
                }
                m_current->put(e, m_next_entry_index++);
            }
			void do_put(const entries &e)
			{
                if (m_current==0)
                    alloc_new_page();
                entries::const_iterator i;
                for (i=e.begin(); i!=e.end(); ++i)
                    put_entry(*i);
			}
		};

		typedef basic_memory_logsink<char> memory_logsink;
		typedef basic_memory_logsink<wchar_t> wmemory_logsink;

    }
}

#ifdef _MSC_VER
#pragma once
#endif

#endif // litwindow_logger_h__151208
