#ifndef litwindow_logger_h__151208
#define litwindow_logger_h__151208

#define LITWINDOW_LOGGER_MUTEX
#define LITWINDOW_LOGGER_HASHMAP

#ifdef LWBASE_EXPORTS
#define LITWINDOW_LOGGER_EXPORTS
#elif defined(LITWINDOW_ALL_DYN_LINK) || defined(LWBASE_DYN_LINK)
#define LITWINDOW_LOGGER_DYN_LINK
#endif

#ifdef LITWINDOW_LOGGER_EXPORTS
#define LITWINDOW_LOGGER_API _declspec(dllexport)
#elif defined(LITWINDOW_LOGGER_DYN_LINK)
#define LITWINDOW_LOGGER_API _declspec(dllimport)
#else
#define LITWINDOW_LOGGER_API
#endif

#include <iostream>
#ifdef LITWINDOW_LOGGER_HASHMAP
#include <hash_map>
#endif
#ifdef LITWINDOW_LOGGER_MUTEX
#include <boost/thread/mutex.hpp>
#endif

namespace litwindow {
	/// encapsules the litwindow logging library
	namespace logger {

		//---------------------------------------------------------------------------------------------
		// 
		namespace details {
			static const size_t word_alignment = sizeof (unsigned long);
			template <typename _Val>
			inline _Val *aligned_ptr(void *p)
			{
				char *ptr=reinterpret_cast<char*>(p);
				size_t misalign= (ptr-reinterpret_cast<char*>(0)) % word_alignment;
				return reinterpret_cast<_Val*>( ptr + (misalign ? word_alignment-misalign : 0));
			}
			template <typename _Val>
			inline const _Val *aligned_ptr(const void *p)
			{
				const char *ptr=reinterpret_cast<const char*>(p);
				size_t misalign= (ptr-reinterpret_cast<const char*>(0)) % word_alignment;
				return reinterpret_cast<const _Val*>( ptr + (misalign ? word_alignment-misalign : 0));
			}
			template <typename _Elem>
			struct defaults
			{
				typedef size_t index_type;
				typedef std::basic_string<_Elem> string_type;
#if defined(LITWINDOW_LOGGER_HASHMAP) 
				typedef stdext::hash_map<string_type, index_type> basic_name_map;
#else
				typedef map<string_type, index_type> basic_name_map;
#endif
#if defined(_MT) && defined(LITWINDOW_LOGGER_MUTEX)
				typedef boost::mutex mutex_type;
				template <typename _L>
				struct mutex_lock_type:public boost::lock_guard<_L>
				{
					mutex_lock_type(_L &l):boost::lock_guard<_L>(l) {}
				};
#else
				struct mutex_type
				{
					void lock() {}
					void unlock() {}
				};
				template <typename _Lock>
				struct mutex_lock_type
				{
					mutex_lock_type(_Lock &){}
				};
#endif
			};
			template <typename _Elem>
			inline _Elem sep();
			template<>
			inline char sep<char>() { return '\t'; }
			template <>
			inline wchar_t sep<wchar_t>() { return L'\t'; }

		}

		//---------------------------------------------------------------------------------------------
		// 
		/// basic_name is a small, lightweight class that contains names for sources and topics.
		/// The names are stored in a global string table and are associated with a numerical index.
		/// Every instance of basic_name searches the global string table for the name passed.
		/// If the name is contained in the string table, it will be reused. Otherwise it will be inserted.
		/// Once inserted it will never be removed.
		/// The indexes are guaranteed to be stable during one application run.
		template <typename _Elem, 
			typename _Index=typename details::defaults<_Elem>::index_type, 
			typename _Lock=typename details::defaults<_Elem>::mutex_type, 
			typename _Container=typename details::defaults<_Elem>::basic_name_map >
		class basic_name
		{
		public:
			typedef basic_name<_Elem, _Index, _Lock, _Container> _Myt;
			typedef _Container container_type;
			typedef _Index index_type;
			typedef typename container_type::key_type name_type;
			typedef std::vector<typename container_type::const_iterator> index_container_type;

			typedef _Index const_iterator;
			/// Construct a basic_name from a char string
			basic_name(const _Elem *n)
			{
				set(name_type(n));
			}
			/// Construct a basic_name from a string, usually a basic_string<_Elem>
			basic_name(const name_type &n)
			{
				set(n);
			}
			/// Construct a basic_name from an index. The index must exist. The constructor
			/// throws an error if the index does not exist.
			basic_name(_Index idx)
			{
				set(idx);
			}

			///\return this names index
			_Index index() const { return m_index; }
			///\return this names string
			const name_type &str() const { return *find_by_index(index()); }
			static _Elem name_sep() { return _Elem(0); }
			///\addtogroup Operators
			//{
			bool operator== (const _Myt &right) const   {return index()==right.index();}
			bool operator!= (const _Myt &right) const   {return !operator==(right);}
			bool operator<  (const _Myt &right) const   {return str()<right.str();}
			bool operator<= (const _Myt &right) const   {return str()<=right.str();}
			bool operator>  (const _Myt &right) const   {return str()>right.str();}
			bool operator>= (const _Myt &right) const   {return str()>=right.str();}
			_Myt operator+  (const _Myt &right) const   {return _Myt(str()+name_sep()+right.str());}
			//}
			///\return an iterator pointing to the beginning of the string table
			static const_iterator begin()
			{
				return const_iterator(0);
			}
			///\return an index beyond the last valid index
			static const_iterator end()
			{
				return const_iterator(size());
			}
		protected:
			void set(const name_type &n)
			{
				typename container_type::const_iterator i=find_name(n);
				m_index=i->second;
			}
			static _Lock &g_lock()
			{
				static _Lock theLock;
				return theLock;
			}
			static typename container_type::iterator find_name(const name_type &n)
			{
				details::defaults<_Elem>::mutex_lock_type<_Lock> lock(g_lock());
				static _Index g_next_free_index=0;
				std::pair<typename container_type::iterator, bool> i=name_container().insert(std::make_pair(n, g_next_free_index));
				if (i.second) {
					index_container().insert(index_container().end(), 1, i.first);
					++g_next_free_index;
				}
				return i.first;
			}

			void set(_Index idx)
			{
				const name_type *n=find_by_index(idx);
				if (n)
					m_index=idx;
				else
					throw std::out_of_range("logger::basic_name - no name registered for index");
			}
			static const name_type *find_by_index(_Index idx)
			{
				const name_type *rc=0;
				g_lock().lock();
				if (idx<index_container().size()) {
					rc=&index_container()[idx]->first;
				}
				g_lock().unlock();
				return rc;
			}
			static _Index size()
			{
				_Index rc;
				g_lock().lock();
				rc=index_container().size();
				g_lock().unlock();
				return rc;
			}
			static container_type LITWINDOW_LOGGER_API &name_container()
			{
				static container_type theNames;
				return theNames;
			}
			static index_container_type LITWINDOW_LOGGER_API &index_container()
			{
				static index_container_type theIndex;
				return theIndex;
			}
			index_type          m_index;
		};

		template <>
		char basic_name<char>::name_sep() { return '/'; }
		template <>
		wchar_t basic_name<wchar_t>::name_sep() { return L'/'; }

		template <typename _Elem>
		std::basic_ostream<_Elem> &operator<<(std::basic_ostream<_Elem> &o, const basic_name<_Elem> &n)
		{
			return o << n.str();
		}
		typedef basic_name<char> name;
		typedef basic_name<wchar_t> wname;

		template <typename _Elem>
		inline const basic_name<_Elem> &default_component();
		template <>
		inline const basic_name<char> &default_component() { static basic_name<char> g_default(""); return g_default; }
		template <>
		inline const basic_name<wchar_t> &default_component() { static basic_name<wchar_t> g_default(L""); return g_default; }
		template <typename _Elem>
		inline const basic_name<_Elem> &default_topic();
		template <>
		inline const basic_name<char> &default_topic() { static basic_name<char> g_default(""); return g_default; }
		template <>
		inline const basic_name<wchar_t> &default_topic() { static basic_name<wchar_t> g_default(L""); return g_default; }

		//---------------------------------------------------------------------------------------------
		/// basic_instance records information about the instance of a log object
		/// It stores the thread id and a unique counter. When transferred between processes or machines
		/// the transfer mechanism adds the process id and, if applicable, the machine id.
		template <typename _Elem>
		class basic_instance
		{
		public:
			struct thread_process_id
			{
				std::basic_string<_Elem> m_name;
				unsigned int	m_thread_id;
				unsigned int	m_process_id;
			};
			static const thread_process_id *get_thread_process_id() { return 0; }
			basic_instance()
				:m_instance(next_instance_value())
				,m_source_id(get_thread_process_id())
			{}
			unsigned int	index() const { return m_instance; }
		private:
			static int next_instance_value()
			{
				static int g_instance=0;
				return g_instance++;
			}
			unsigned int m_instance;
			const thread_process_id *m_source_id;
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
				information,
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
				case information: return "information";
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
				case information: return L"information";
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
		template <typename _Elem> class basic_logsink;
		template <typename _Elem> inline basic_logsink<_Elem> *default_sink();

		typedef basic_logsink<char> logsink;
		typedef basic_logsink<wchar_t> wlogsink;

		/// basic_logbuf is a streambuf accepting events and storing them in a logsink
		template <typename _Elem, typename _Traits, typename _Alloc >
		class basic_logbuf:public std::basic_streambuf<_Elem, _Traits>, private boost::noncopyable
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
			typedef basic_logsink<_Elem>	sink_type;
			typedef basic_name<_Elem>		name_type;
			typedef basic_instance<_Elem>	instance_type;
			typedef time_t timestamp_type;
		private:
			struct entry
			{
				timestamp_type  m_timestamp;
				unsigned long	m_index;
				unsigned int    m_instance;
				unsigned short  m_component;
				unsigned short  m_topic;
				unsigned short  m_level;
				unsigned short  m_length;
				size_t          header_size() const { return reinterpret_cast<const char*>(this+1)-reinterpret_cast<const char*>(this); }
				const _Elem     *begin_data() const { return reinterpret_cast<const _Elem*>(this+1); }
				const _Elem     *end_data() const { return begin_data()+length(); }
				const _Elem	   *raw_data() const { return begin_data(); }
				size_t			length() const { return m_length; }
				std::basic_string<_Elem> str() const { return std::basic_string<_Elem>(raw_data(), length()); }
				basic_name<_Elem> component() const { return basic_name<_Elem>(m_component); }
				basic_name<_Elem> topic() const { return basic_name<_Elem>(m_topic); }
				size_t			level() const { return m_level; }
				void			index(size_t new_index) { m_index=new_index; }
				size_t			index() const { return m_index; }
				timestamp_type	timestamp() const { return m_timestamp; }
				size_t          full_size_in_bytes() const 
				{
					return reinterpret_cast<const char*>(end_data())-reinterpret_cast<const char*>(this); 
				}
				entry *next_entry() const
				{
					return const_cast<entry*>(details::aligned_ptr<entry>(end_data()));
					//const unsigned char *next=reinterpret_cast<const unsigned char *>(end_data());
					//size_t sz=next-reinterpret_cast<const unsigned char*>(this);
					//size_t align=sizeof(unsigned long);
					//size_t misalign=sz%align;
					//if (misalign) {
					//	next=next + align-misalign;
					//}
					//return const_cast<entry*>(reinterpret_cast<const entry*>(next));
				}
			};
			class entries
			{
			public:
				typedef typename basic_logbuf<_Elem, _Traits, _Alloc>::entry entry;
				entries(const char_type *begin, const char_type *end)
					:m_begin(begin), m_end(end)
				{
				}
				class const_iterator
				{
					const entry *m_ptr;
					const entry *get_entry() const { return m_ptr; }
				public:
					const_iterator(const char_type *p=0)
						:m_ptr(reinterpret_cast<const entry*>(p)){}
					const_iterator(const const_iterator &i)
						:m_ptr(i.m_ptr){}
					const const_iterator &operator++()
					{
						const entry *next=get_entry()->next_entry();
						m_ptr=next;
						return *this;
					}
					bool operator==(const const_iterator &r) const { return m_ptr==r.m_ptr; }
					bool operator!=(const const_iterator &r) const { return !operator==(r); }
					const const_iterator &operator=(const const_iterator &i) { m_ptr=i.m_ptr; return *this; }
					//const char_type *raw_data() const { return get_entry()->raw_data(); }
					//size_t length() const { return get_entry()->length(); }
					//std::basic_string<char_type> str() const { return std::basic_string<char_type>(raw_data(), raw_data()+length()); }
					const entry *operator->() const { return get_entry(); }
					const entry &operator*() const { return *get_entry(); }
				};
				const_iterator begin() const { return const_iterator(m_begin); }
				const_iterator end() const { return const_iterator(m_end); }
			private:
				const char_type	*m_begin;
				const char_type *m_end;
			};
			enum {
				GROW_INCREMENT = 4096   ///< number of elements the log buffer is grown per overflow request
			};
			void    init()
			{
				memset(&m_entry, 0, sizeof(m_entry));
				m_state=0;
				m_sink=default_sink<_Elem>();
				m_sync_frequency=1;	// sync after every log entry
				m_sync_period=0;
				m_log_count_since_last_sync=0;
				m_next_index=0;
				m_time_of_last_sync=timestamp();
				clear();
			}
			entry	m_entry;
			template <typename Value>
			void do_put(const Value &v)
			{
				sputn((const char_type*)&v, (sizeof(v)+sizeof(char_type)-1)/sizeof(char_type));
			}
			size_t count() const { return pptr()-m_begin_data; }
			timestamp_type timestamp() const { return time(0); }

			char_type	*m_begin_data;
			entry		*m_current_entry;
			sink_type	*m_sink;
			allocator_type m_allocator;
			_Strstate m_state;
			int_type end_of_log_entry;
			size_t		m_sync_frequency;
			timestamp_type m_sync_period;
			size_t		m_log_count_since_last_sync;
			timestamp_type m_time_of_last_sync;
			size_t		m_next_index;
			bool		need_sync() const
			{
				return m_sync_frequency>0 && m_log_count_since_last_sync>=m_sync_frequency
					|| m_sync_period>0 && m_current_entry && m_current_entry->m_timestamp-m_time_of_last_sync>=m_sync_period;
			}
		public:
			basic_logbuf():end_of_log_entry(0) { init(); }
			~basic_logbuf() { clear(); sink(0); }
			void sink(sink_type *new_sink)			{ m_sink=new_sink; }
			sink_type *sink() const					{ return m_sink; }
			/// specify flush frequency: write to sink after \p log entries
			void sync_frequency(size_t f)			{ m_sync_frequency=f; }
			/// specify flush period: write to sink after a timespan of \p p
			void sync_period(timestamp_type p)		{ m_sync_period=p; }

			void    level(size_t lvl)				{ m_current_entry->m_level=lvl; }
			size_t  level() const					{ return m_current_entry->m_level; }
			void    topic(const name_type &t)		{ m_current_entry->m_topic=t.index(); }
			size_t  topic() const					{ return m_current_entry->m_topic; }
			void	component(const name_type &c)	{ m_current_entry->m_component=c.index(); }
			size_t	component() const				{ return m_current_entry->m_component; }
			void	instance(const instance_type &i){ m_current_entry->m_instance=i.index(); }

			virtual int sync()
			{
				m_log_count_since_last_sync=0;
				m_time_of_last_sync=m_current_entry ? m_current_entry->m_timestamp : timestamp();
				int rc=_Mysb::sync();
				//TODO: Optimize this
				if (sink()) {
					sink()->put(entries(_Mysb::pbase(), _Mysb::pptr()));
					_Mysb::setp(_Mysb::pbase(), _Mysb::pbase(), _Mysb::epptr());
					m_begin_data=0;
					m_current_entry=0;
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
					std::_Traits_helper::copy_s<_Traits>(_Newptr, _Newsize, _Oldptr, _Oldsize);
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
			void clear()
			{
				if (m_state & _Allocated) {
					m_allocator.deallocate(_Mysb::pbase(), _Mysb::epptr()-_Mysb::pbase());
					m_state&=~_Allocated;
				}
				_Mysb::setp(0,0);
				m_current_entry=0;
				m_begin_data=0;
			}
			void begin_entry()
			{
				m_entry.m_timestamp=timestamp();
				m_entry.index(m_next_index++);
				if (m_current_entry==0) {
					do_put(m_entry);
					m_begin_data=pptr();
					m_current_entry=((entry*)m_begin_data)-1;
				}
			}
			void end_entry()
			{
				if (m_current_entry) {
					off_t total=count();
					if (total>0) {
						++m_log_count_since_last_sync;
						m_current_entry->m_length=(unsigned short)total;
						m_current_entry=0;
						// pad to fill word alignment
						_Elem *next_free=details::aligned_ptr<_Elem>(pptr());
						size_t padding=next_free-pptr();
						while (padding--)
							sputc(0);
						if (need_sync())
							sync();
					}
				}
			}
		};

		template <typename _Elem, typename _Traits=std::char_traits<_Elem>, typename _Alloc=std::allocator<_Elem> >
		class basic_logstream:public std::basic_ostream<_Elem, _Traits>
		{
			typedef std::basic_ostream<_Elem, _Traits> inherited;
		public:
			typedef basic_logbuf<_Elem, _Traits, _Alloc> _Streambuf;
			typedef basic_logstream<_Elem, _Traits> _Myt;
			typedef typename _Streambuf::sink_type sink_type;
			typedef basic_name<_Elem> name_type;
			typedef _Myt& (*logmanipulator)(_Myt&);
			basic_logstream()
				:inherited(&m_rdbuf)
			{
			}
			_Streambuf *rdbuf() { return &m_rdbuf; }
			void put_level(levels::default_level_enum l)
			{
			}
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
			_Myt &put_topic(const name_type &topic)
			{
				m_rdbuf.topic(topic); return *this;
			}
			_Myt &put_component(const name_type &component)
			{
				m_rdbuf.component(component); return *this;
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
			typedef typename stream_type::char_type char_type;
			typedef basic_name<char_type> name_type;
			void put_level(_Stream &stream, levels::default_level_enum l) { }
			void set_sink(_Stream &stream, basic_logsink<char_type> *s) { }
			void lbegin(_Stream &stream) { }
			void lend(_Stream &stream) { }

			void put_component(_Stream &stream, const name_type &c) { stream << c.str() << details::sep<char_type>(); }
			void put_topic(_Stream &stream, const name_type &t) { stream << t.str() << details::sep<char_type>(); }
		};
		template <typename _Elem>
		struct stream_traits<basic_logstream<_Elem> >
		{
			typedef basic_logstream<_Elem> stream_type;
			typedef typename stream_type::char_type char_type;
			typedef basic_name<char_type> name_type;
			void put_level(stream_type &stream, levels::default_level_enum l) { stream.put_level(l); }
			void set_sink(stream_type &stream, typename stream_type::sink_type *s) { stream.sink(s); }
			void lbegin(stream_type &stream) { stream.begin_entry(); }
			void lend(stream_type &stream) { stream.end_entry(); }
			void put_component(stream_type &stream, const name_type &c) { stream.put_component(c); }
			void put_topic(stream_type &stream, const name_type &t) { stream.put_topic(t); }
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
			typedef basic_name<_Elem> name_type;
			typedef basic_logsink<_Elem> sink_type;
			typedef basic_instance<_Elem> instance_type;
			bool m_enabled;
			_Streamtraits m_stream_traits;
		public:
			class inserter
			{
				const inserter &operator=(const inserter &i);
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
				inserter(const inserter &i)
					:_enabled(i._enabled)
					,_owner(i._owner)
				{
					_owner.do_begin();
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


			basic_events(const name_type &component=default_component<_Elem>(), const name_type &topic=default_topic<_Elem>(), sink_type &target=*default_sink<_Elem>())
				:m_default_component(component)
				,m_default_topic(topic)
				,m_enabled(true)
				,m_ignore_begin_end_count(0)
				,m_open_count(0)
			{
				sink(&target);
			}
			basic_events(sink_type &target)
				:m_default_component(default_component<_Elem>())
				,m_default_topic(default_topic<_Elem>())
				,m_enabled(true)
				,m_ignore_begin_end_count(0)
				,m_open_count(0)
			{
				sink(&target);
			}
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
			void sink(sink_type *s)
			{
				m_stream_traits.set_sink(stream(), s);
			}

			void component(const name_type &c) 
			{
				m_default_component=c;
				m_stream_traits.put_component(stream(), c);
			}
			void topic(const name_type &t)
			{
				m_default_topic=t;
				m_stream_traits.put_topic(stream(), t);
			}
			void continue_entry()
			{
				// ignore one 'end' and one 'begin'
				m_ignore_begin_end_count=2;
			}
		private:
			size_t	m_ignore_begin_end_count;
			size_t	m_open_count;
			/// begin a new entry
			void do_begin()
			{
				if (m_open_count++==0) {
					if (m_ignore_begin_end_count==0) {
						m_stream_traits.lbegin(*this);
						component(m_default_component);
						topic(m_default_topic);
					} else
						--m_ignore_begin_end_count;
				}
			}
			/// end an entry
			void do_end()
			{
				if (m_open_count==1) {
					if (m_ignore_begin_end_count==0) {
						m_stream_traits.lend(*this); 
					} else
						--m_ignore_begin_end_count;
					m_open_count=0;
				} else if (m_open_count>0)
					--m_open_count;
			}
			friend class inserter;
			instance_type m_instance;
			name_type	m_default_component;
			name_type	m_default_topic;
		};

		template <typename _Elem, typename Streambuf>
		inline basic_events<_Elem, Streambuf> &debug(basic_events<_Elem, Streambuf> &in) { in.level(levels::debug_level); return in; }
		template <typename _Elem, typename Streambuf>
		inline basic_events<_Elem, Streambuf> &information(basic_events<_Elem, Streambuf> &in) { in.level(levels::information); return in; }
		template <typename _Elem, typename Streambuf>
		inline basic_events<_Elem, Streambuf> &info(basic_events<_Elem, Streambuf> &in) { return information(in); }
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

		template <typename _Elem, typename Streambuf>
		inline basic_events<_Elem, Streambuf> &contl(basic_events<_Elem, Streambuf> &in) { in.continue_entry(); return in; }

		typedef basic_events<char> events;
		typedef basic_events<wchar_t> wevents;
		typedef events log;
		typedef wevents wlog;

		/*!\page litwindow_logger Logging: recording events and states.
		* Copyright 2009 Hajo Kirchhoff, Lit Window Productions - Kirchhoff-IT Consulting, www.litwindow.com
		* \section logging_intro Introduction and Overview

		* Litwindow.logging is a library for generating and recording information about
		* a programs runtime environment and execution. The methods and procedures presented
		* here are commonly refered to as 'logging'. The library aims to give programmers a 
		* simple, flexible and scalable framework to 'log' information for diagnostics,
		* debugging and UI purposes and collect the information during program execution.

		* - \subpage logger_quick_intro gives you some code samples and a very short introduction

		* \subsection rationale Rationale
		* Logging is often thought of as a slightly improved 'printf' with which one peppers the 
		* source code with in hope of finding that problematic bug. More experienced software
		* developers however see logging in a greater perspective, especially if they have ever
		* had to diagnose defects that only occurred under very specific circumstances and only
		* on the customers system. As systems become more complex, good diagnostics information
		* is ever more important. This library treats the task of logging as an integral part of software development.

		* \subsection design_philosophy Design philosophy
		* The library distinguishes between logging events and logging states.

		* \par Events
		* Events essentially say: "The program execution reached this point at that time with the following information".
		* The classical log file is simply a list of events, usually in the order in which they occurred.

		* 		Events are characterized by several attributes.
		* 		<table>
		* 		<tr><td>Time<td>When did the event occur?
		* 		<tr><td>Level<td>What level (severity) does the event have?
		* 		<tr><td>Component<td>What (software) component reported the event?
		* 		<tr><td>Topic<td>What kind of information does it contain?
		* 		<tr><td>Instance<td>Where in the runtime environment did it happen?
		* 		<tr><td>Id<td>A unique identifier of the event
		* 		<tr><td>Text<td>The actual text of the event
		* 		</td></tr></table>

		* 	\par States
		*	States are very similar to events, with one distiction. While events have no memory - they are 
		* 	simply recorded and forgotten - states have a 'current' value. The collection of states can give
		* 	a programmer or a user information about the state the program is in.
		*
		* \par Sinks and filters
		* All logging information is written to a logging stream, which is connected to a sink. A sink combines
		* the output from several separate streams into one. Sinks can be chained and multiple sinks can be
		* connected to a single master sink. Filters reduce the amount of information that is passed into a sink.
		*
		* \par Muting and deactivating log information
		* Events and states can be selectively disabled at runtime so that more chatty
		* logging sources can be muted until their information is actually helpful.
		* Or they can be completely deactivated at compile time so that no code is generated for them, but 
		* their respective logging statements remain in the source code, should they be needed again.

		* \subsection logging_design Design goals
		* -# simplicity
		* 	- Inexperienced programmers shall be able to learn it within minutes.
		* 	- Logging program events and states shall be possible without constantly looking up the documentation.
		* -# flexibility
		* 	- Allow programmers to adapt to their specific programming scenario.
		* 	- Shall selectively en- or disable programmer specified parts of logging information.
		* 		- This shall be possible at compile time and at runtime.
		* -# scalability\n
		* 		Shall be suitable for ...
		* 	- single thread command line tools
		* 	- single process, multi-threaded GUI applications
		* 	- multi-process, multi-homed distributed systems (client-server)
		* -# performance
		* 	- Must work in multithreaded environments.
		* 	- Have very low logging overhead.
		* 	- Have the ability to defer more costly operations to a low priority thread.
		* -# cleanliness\n
		* 		Traditional logging mechanisms make heavy use of macros and sometimes require unorthodox
		* 		calling conventions. This tends to obfuscate the source code or interrupt readability.
		* 		Also macros come with their own set of problems (namespace collisions, side effects).
		* 		The library presented here avoids the use of macros alltogether.
		* \subsection library_architecture Architecture
		* The library is split into two parts:
		* \par generating event and state information
		* This part is used by all programmers in their source code to report information.
		* 	- \ref basic_events and basic_states are streams to report information.
		* 	- basic_level, basic_component, basic_topic, basic_instance and basic_id encapsulate
		* 		the attributes that describe an event or state.
		* 	- basic_tag is a low-level class implementing a global string table used by basic_component
		* 		and other classes.

		* \par collecting and storing event and state information
		* This part will be used only by some programmers to collect and possibly store the information 
		* generated by the other part of the library.
		* 	- basic_logbuf and basic_logstream are used by basic_events and basic_states to compile
		* 		event and state information and send them to a basic_logsink.
		* 	- basic_logsink collects information from logging sources. It is thread safe
		* 		and combines several logging sources into one stream.
		* 	- basic_memory_logsink, basic_file_logsink and others store the logging information and
		* 		have an interface to query the logging information.
		*/

		/*! \page logger_quick_intro Logger: A quick introduction
		* This section gives you a quick introduction.
		* \section quick_intro_logging Logging in the source code.
		* Use events and wevents, derived from basic_events much as you would use an std::stream.
		* This example writes logging information to the global log sink, which needs to be
		* initialized elsewhere.

		* \par Simple case, no component or topic
		* \code
		* #include <litwindow/logger.hpp>
		* using namespace litwindow::logger;
		*
		* // define an events object
		* events ol;
		* // write to the events object
		* ol << "This gets written to the log";	// default level
		* ol << logger::error << "This is an error";	// error level
		* \endcode
		* The output in a simple log file would look like this.
		* \verbatim
12:52	info	This gets written to the log
12:53	error	This is an error \endverbatim

		* \par Using components and topics, Part I
		* \code
		* class my_sub_system
		* {
		*    logger::events ol_connect, ol;	// declare two events streams
		*
		*	 // initialise them and set up component and topic
		*    my_sub_system():ol_connect("communication.subsystem1", "connections")
		*    {
		*        // or change the component after initialisation
		*        ol.component("communication.subsystem1");
		*    }
		*    void connect_established(size_t portNo)
		*    {
		*        // log information
		*        ol_connect << "connection established at port: " << portNo;
		*    }
		* };
		* \endcode
		* \par Using components and topics, Part II
		* \code
		* // define two components
		* component com_sub1("communication.subsystem1");
		* component flat("flat");
		* // define a topics
		* topic t_connections("connections");
		* events ol;
		*
		* ol && com_sub1 && t_connections && "connection established at port: " && port_no;
		*
		* ol && logger::debug && flat && "This uses the default topic";
		* \endcode
		* This is what the output including component and topic would look like.
		* \verbatim
13:10	info	communication.subsystem1	connections	connection established at port: 9731
13:13	debug0	flat						This uses the default topic.\endverbatim
		*
		* \par Deactivate a log stream but leave its statement in the source code.
		* \code
		* // Change events to deactivated_events
		* deactivated_events ol;
		* ol && com_sub1 && t_tcp && "connection established at port: " && port_no;
		* \endcode
		* \note The compiler will not generate any code for this statement in release builds. deactivated_events
		* are implicitly convertible to bool and will always return false. Thus the statement will be
		* false && com_sub1 && t_tcp ... which always evaluates to false and will be short-circuited. The compiler
		* will optimize this and remove the entire statement.
		* \note This is the reason why logging streams (events or states) define the && operator to stream
		* information. The << operator is used for compatibility with iostreams but does not have the capability
		* to deactivate the log stream at compile time.
		* \section quick_intro_sink Specifying and using sinks.
		* \par Using a stringstream as the destination.
		* - define a stringstream
		* - define a logsink with the stringstream as its target
		* - modify the formatter
		* - define an events object with the logsink as its target
		* - write event information to the events object
		* - query the stringstream for the output
		*.
		* \code
		* std::wstringstream s;
		* wostream_logsink sink(s);
		* sink.format().timestamp=false;	// no timestamp
		* sink.format().level=false;		// no level
		* wevents e(&sink);
		* e.sink(&sink);
		* e && debug && L"This is a test with number " && 800;
		* e && warning && L"Some more tests.";
		* wstring rc(s.str());
		* BOOST_CHECK(rc==wstring(L"\t\tThis is a test with number 800\n\t\tSome more tests.\n"));
		* \endcode
		*/
	}
}

#include "logger/sink.hpp"

#ifdef _MSC_VER
#pragma once
#endif

#endif // litwindow_logger_h__151208

