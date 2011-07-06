#ifndef litwindow_logger_h__151208
#define litwindow_logger_h__151208

/// define this to use boost::mutex
#define LITWINDOW_LOGGER_MUTEX
/// define this to use hashmap for basic_tag
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
#include <boost/thread/tss.hpp>

#endif
#include <boost/lexical_cast.hpp>

#ifdef max
#undef max
#endif
#include <limits>

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
		///\brief Small, lightweight class containing strings as tags for basic_component, basic_level, basic_topic, basic_instance.
		///
		/// The strings are stored in a global string table and associated with a unique numerical value.
		/// Every instance of basic_tag searches the global string table for its string parameter.
		/// If the string is contained in the string table, it will be reused. Otherwise it will be inserted.
		/// Once inserted it will never be removed.
		///
		/// The indexes are guaranteed to be stable during one application run.
		/// \threadsafe
		template <typename _Elem, 
			typename _Index=typename details::defaults<_Elem>::index_type, 
			typename _Lock=typename details::defaults<_Elem>::mutex_type, 
			typename _Container=typename details::defaults<_Elem>::basic_name_map >
		class basic_tag
		{
			typedef basic_tag<_Elem, _Index, _Lock, _Container> _Myt;
			typedef _Container container_type;
			typedef typename container_type::key_type name_type;
			typedef std::vector<typename container_type::const_iterator> index_container_type;
		public:
			typedef _Index index_type;

			typedef _Index const_iterator;
			/// Default constructs an empty tag
			basic_tag():m_index(std::numeric_limits<index_type>::max()){}
			/// Construct from a pointer to char
			basic_tag(const _Elem *n)
			{
				set(name_type(n));
			}
			/// Construct from a string, usually a basic_string<_Elem>
			basic_tag(const name_type &n)
			{
				set(n);
			}
			/// Construct from an index.
			///
			///\throw an error if the index does not exist.
			basic_tag(_Index idx)
			{
				set(idx);
			}

			///\return this names index
			_Index index() const { return m_index; }
			///\return this names string
			const name_type &str() const { return *find_by_index(index()); }
			static _Elem tag_sep() { return _Elem(0); }
			///\name Operators
			//{
			bool operator== (const _Myt &right) const   {return index()==right.index();}
			bool operator!= (const _Myt &right) const   {return !operator==(right);}
			bool operator<  (const _Myt &right) const   {return str()<right.str();}
			bool operator<= (const _Myt &right) const   {return str()<=right.str();}
			bool operator>  (const _Myt &right) const   {return str()>right.str();}
			bool operator>= (const _Myt &right) const   {return str()>=right.str();}
			_Myt operator+  (const _Myt &right) const   {return _Myt(str()+tag_sep()+right.str());}
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
			static _Lock LITWINDOW_LOGGER_API &g_lock()
			{
				static _Lock theLock;
				return theLock;
			}
			static typename container_type::iterator find_name(const name_type &n)
			{
				details::defaults<_Elem>::mutex_lock_type<_Lock> lock(g_lock());
				container_type &c(name_container());
				std::pair<typename container_type::iterator, bool> i=c.insert(std::make_pair(n, c.size()));
				if (i.second) {
					index_container().insert(index_container().end(), 1, i.first);
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
		char basic_tag<char>::tag_sep() { return '/'; }
		template <>
		wchar_t basic_tag<wchar_t>::tag_sep() { return L'/'; }

		template <typename _Elem>
		std::basic_ostream<_Elem> &operator<<(std::basic_ostream<_Elem> &o, const basic_tag<_Elem> &n)
		{
			return o << n.str();
		}
		typedef basic_tag<char> tag;
		typedef basic_tag<wchar_t> wtag;

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
			explicit basic_instance(unsigned int i)
				:m_instance(i){}
			unsigned int	index() const { return m_instance; }
			std::basic_string<_Elem> str() const { return boost::lexical_cast<std::basic_string<_Elem> >(index()); }
		private:
			static int next_instance_value()
			{
				static int g_instance=0;
				return g_instance++;
			}
			unsigned int m_instance;
			const thread_process_id *m_source_id;
		};

		template <typename Category, typename _Elem>
		class basic_tag_with_category:public basic_tag<_Elem>
		{
			typedef basic_tag<_Elem> inherited;
		public:
			basic_tag_with_category():inherited(){}
			basic_tag_with_category(const _Elem *s):inherited(s){}
			basic_tag_with_category(const std::basic_string<_Elem> &s):inherited(s){}
			basic_tag_with_category(typename inherited::index_type i):inherited(i){}
		};

		namespace {
            struct cat_component {};
            struct cat_topic {};
            struct cat_level {};
            struct cat_instance {};
		};
		template <typename _Elem>
		struct basic_component:public basic_tag_with_category<cat_component, _Elem>
		{
			basic_component():basic_tag_with_category<cat_component, _Elem>(){}
			explicit basic_component(const _Elem *s):basic_tag_with_category<cat_component, _Elem>(s){}
			explicit basic_component(const std::basic_string<_Elem> &s):basic_tag_with_category<cat_component, _Elem>(s){}
			explicit basic_component(typename basic_tag_with_category<cat_component, _Elem>::index_type i):basic_tag_with_category<cat_component, _Elem>(i){}
		};
		template <typename _Elem>
		struct basic_topic:public basic_tag_with_category<cat_topic, _Elem>
		{
			basic_topic():basic_tag_with_category<cat_topic, _Elem>(){}
			explicit basic_topic(const _Elem *s):basic_tag_with_category<cat_topic, _Elem>(s){}
			explicit basic_topic(const std::basic_string<_Elem> &s):basic_tag_with_category<cat_topic, _Elem>(s){}
			explicit basic_topic(typename basic_tag_with_category<cat_topic, _Elem>::index_type i):basic_tag_with_category<cat_topic, _Elem>(i){}
		};
		typedef basic_component<char> component;
		typedef basic_component<wchar_t> wcomponent;
		typedef basic_topic<char> topic;
		typedef basic_topic<wchar_t> wtopic;
		template <typename _Elem>
		class basic_level:public basic_tag_with_category<cat_level, _Elem> 
		{
			typedef basic_tag_with_category<cat_level, _Elem> inherited;
		public:
			enum preset {
				unknown,
				critical,
				error,
				warning,
				information,
				info = information,
				debug0,
				debug = debug0,
				debug1,
				debug2,
				debug3,
				debug4,

				end_marker_for_preset_levels
			};
			typedef inherited tag_type;
			inline static const tag_type &get(preset p);
			basic_level(preset p):inherited(get(p)){}
			explicit basic_level(const tag_type &t):inherited(t){}
			explicit basic_level(const _Elem *s):inherited(s){}
			explicit basic_level(const std::basic_string<_Elem> &s):inherited(s){}
		};
		template <>
		inline static const typename basic_level<char>::tag_type &basic_level<char>::get(basic_level<char>::preset p)
		{
			static const tag_type g_tags[]=
			{
				tag_type("unknown"), tag_type("critical"), tag_type("error"), tag_type("warning"), tag_type("info"), 
				tag_type("debug"), tag_type("debug1"), tag_type("debug2"), tag_type("debug3"), tag_type("debug4")
			};
			return g_tags[p<sizeof(g_tags)/sizeof(g_tags[0]) ? p : 0];
		}
		template <>
		inline static const typename basic_level<wchar_t>::tag_type &basic_level<wchar_t>::get(basic_level<wchar_t>::preset p)
		{
			static const tag_type g_tags[]=
			{
				tag_type(L"unknown"), tag_type(L"critical"), tag_type(L"error"), tag_type(L"warning"), tag_type(L"info"), 
				tag_type(L"debug"), tag_type(L"debug1"), tag_type(L"debug2"), tag_type(L"debug3"), tag_type(L"debug4")
			};
			return g_tags[p<sizeof(g_tags)/sizeof(g_tags[0]) ? p : 0];
		}

		typedef basic_level<char> level;
		typedef basic_level<wchar_t> wlevel;

		template <typename _Elem>
		inline const basic_component<_Elem> &default_component();
		template <>
		inline const basic_component<char> &default_component() { static basic_component<char> g_default(""); return g_default; }
		template <>
		inline const basic_component<wchar_t> &default_component() { static basic_component<wchar_t> g_default(L""); return g_default; }
		template <typename _Elem>
		inline const basic_topic<_Elem> &default_topic();
		template <>
		inline const basic_topic<char> &default_topic() { static basic_topic<char> g_default(""); return g_default; }
		template <>
		inline const basic_topic<wchar_t> &default_topic() { static basic_topic<wchar_t> g_default(L""); return g_default; }

		template <typename _Elem>
		inline const basic_level<_Elem> &default_level() { static basic_level<_Elem> g_default(basic_level<_Elem>::info); return g_default; }

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
			typedef basic_level<_Elem> level_type;
			typedef basic_component<_Elem> component_type;
			typedef basic_topic<_Elem> topic_type;
			typedef basic_logsink<_Elem> sink_type;
			typedef basic_instance<_Elem> instance_type;
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
				component_type	component() const { return component_type(m_component); }
				topic_type		topic() const { return topic_type(m_topic); }
				level_type		level() const { return level_type(m_level); }
				instance_type	instance() const { return instance_type(m_instance); }
				void			index(size_t new_index) { m_index=unsigned long(new_index); }
				size_t			index() const { return m_index; }
				timestamp_type	timestamp() const { return m_timestamp; }
				size_t          full_size_in_bytes() const 
				{
					return reinterpret_cast<const char*>(end_data())-reinterpret_cast<const char*>(this); 
				}
				entry *next_entry() const
				{
					return const_cast<entry*>(details::aligned_ptr<entry>(end_data()));
				}
				bool validate() const
				{
#ifdef _DEBUG
					if (length()>4096 || m_timestamp>time(0)+5 || m_index>100000 || m_index>10000 || m_component>10000 || m_topic>10000 || m_level>1000) {
						return false;
					}
#endif
					return true;
				}
			};
			class entries
			{
			public:
				typedef typename basic_logbuf<_Elem, _Traits, _Alloc>::entry entry;
				entries(const char_type *b, const char_type *e)
					:m_begin(b), m_end(e)
				{
					begin()->validate();
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
			entry		*current_entry() { if (m_current_entry==0) begin_entry(); return m_current_entry; }
			void		current_entry(entry * val) { m_current_entry = val; }
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

			void    level(const level_type &lvl)		{ current_entry()->m_level=unsigned short(lvl.index()); }
			size_t  level() const					{ return m_current_entry->m_level; }
			void    topic(const topic_type &t)		{ current_entry()->m_topic=(unsigned short)t.index(); }
			size_t  topic() const					{ return m_current_entry->m_topic; }
			void	component(const component_type &c)	{ current_entry()->m_component=(unsigned short)c.index(); }
			size_t	component() const				{ return m_current_entry->m_component; }
			void	instance(const instance_type &i){ current_entry()->m_instance=i.index(); }
			unsigned int instance() const			{ return m_current_entry->m_instance; }

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
					off_t total=(off_t)count();
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
			typedef basic_level<_Elem> level_type;
			typedef basic_component<_Elem> component_type;
			typedef basic_topic<_Elem> topic_type;
			typedef basic_logsink<_Elem> sink_type;
			typedef basic_instance<_Elem> instance_type;
			typedef _Myt& (*logmanipulator)(_Myt&);
			basic_logstream()
				:inherited(&m_rdbuf)
			{
			}
			_Streambuf *rdbuf() { return &m_rdbuf; }
			const _Streambuf *rdbuf() const { return &m_rdbuf; }
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
			_Myt &put_level(const level_type &lvl)
			{
				m_rdbuf.level(lvl); return *this;
			}
			_Myt &put_topic(const topic_type &topic)
			{
				m_rdbuf.topic(topic); return *this;
			}
			_Myt &put_component(const component_type &component)
			{
				m_rdbuf.component(component); return *this;
			}
			_Myt &put_instance(const instance_type &instance)
			{
				m_rdbuf.instance(instance); return *this;
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
			typedef basic_level<char_type> level_type;
			typedef basic_component<char_type> component_type;
			typedef basic_topic<char_type> topic_type;
			typedef basic_logsink<char_type> sink_type;
			typedef basic_instance<char_type> instance_type;
			void set_sink(_Stream &stream, basic_logsink<char_type> *s) { }
			basic_logsink<char_type> *get_sink(const _Stream &) const { return 0; }
			void lbegin(_Stream &stream) { }
			void lend(_Stream &stream) { }

			void put_level(_Stream &stream, const level_type &l) { stream << l.str() << details::sep<char_type>(); }
			void put_component(_Stream &stream, const component_type &c) { stream << c.str() << details::sep<char_type>(); }
			void put_topic(_Stream &stream, const topic_type &t) { stream << t.str() << details::sep<char_type>(); }
			void put_instance(_Stream &stream, const instance_type &i) { stream << i.str() << details::sep<char_type>(); }
		};
		template <typename _Elem>
		struct stream_traits<basic_logstream<_Elem> >
		{
			typedef basic_logstream<_Elem> stream_type;
			typedef typename stream_type::char_type char_type;
			typedef basic_level<char_type> level_type;
			typedef basic_component<char_type> component_type;
			typedef basic_topic<char_type> topic_type;
			typedef basic_logsink<char_type> sink_type;
			typedef basic_instance<char_type> instance_type;
			void set_sink(stream_type &stream, typename stream_type::sink_type *s) { stream.sink(s); }
			typename stream_type::sink_type *get_sink(const stream_type &stream) const { return stream.sink(); }
			void lbegin(stream_type &stream) { stream.begin_entry(); }
			void lend(stream_type &stream) { stream.end_entry(); }
			void put_level(stream_type &stream, const level_type &l) { stream.put_level(l); }
			void put_component(stream_type &stream, const component_type &c) { stream.put_component(c); }
			void put_topic(stream_type &stream, const topic_type &t) { stream.put_topic(t); }
			void put_instance(stream_type &stream, const instance_type &i) { stream.put_instance(i); }
		};
		// ---------------------------------------------------------------------------------------------

        enum enable_state { enabled, disabled };
		/// Base class for logging events
		template <
			typename _Elem,
			typename _Outstream=basic_logstream<_Elem> ,
			typename _Streamtraits=logger::stream_traits<_Outstream>
		>
		class basic_events:public _Outstream
		{
		public:
			typedef _Outstream Inherited;
			typedef basic_events<_Elem, _Outstream, _Streamtraits> _Myt;
			typedef _Elem char_type;
			typedef _Outstream outstream_type;
			typedef _Streamtraits outstream_traits;
			typedef std::basic_ostream<char_type>& (*iomanipulator)(std::basic_ostream<char_type>&);
			typedef _Myt& (*logmanipulator)(_Myt&);
			typedef basic_tag<char_type> tag_type;
			typedef basic_level<char_type> level_type;
			typedef basic_component<char_type> component_type;
			typedef basic_topic<char_type> topic_type;
			typedef basic_logsink<char_type> sink_type;
			typedef basic_instance<char_type> instance_type;
		private:
			bool m_enabled;
			outstream_traits m_stream_traits;
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
					//if (pFn==std::endl) {

					//}
					////(*pFn)(_owner._outstream);
					return *this;
				}
				template <typename Value>
				inserter &operator <<(const Value &v)
				{
					return operator&&<Value>(v);
				}
				inserter &operator<<(std::basic_ostream<_Elem> &(*pFn)(std::basic_ostream<_Elem> &))
				{
					return operator&&(pFn);
				}
			};


			basic_events(
				const component_type &c=default_component<char_type>(),
				const topic_type &t=default_topic<char_type>(),
                enable_state is_enabled=logger::enabled,
				sink_type &target=*default_sink<char_type>())
				:
			m_default_component(c), m_component(m_default_component)
				,m_default_topic(t), m_topic(m_default_topic)
				,m_default_level(default_level<char_type>()), m_level(m_default_level)
				,m_enabled(is_enabled!=disabled)
				,m_ignore_begin_end_count(0)
				,m_open_count(0)
			{
				sink(&target);
			}
			basic_events(
				const char_type *c,
				const char_type *t=default_topic<_Elem>().str().c_str(),
                enable_state is_enabled=logger::enabled,
				sink_type &target=*default_sink<char_type>()
				)
				:m_default_component(c), m_component(m_default_component)
				,m_default_topic(t), m_topic(m_default_topic)
				,m_default_level(default_level<char_type>()), m_level(m_default_level)
				,m_enabled(is_enabled!=disabled)
				,m_ignore_begin_end_count(0)
				,m_open_count(0)
			{
				reset_tags_to_default();
				sink(&target);
			}
			basic_events(const basic_events &e)
				:m_default_component(e.m_default_component), m_component(e.m_component)
				,m_default_topic(e.m_default_topic), m_topic(e.m_topic)
				,m_default_level(e.m_default_level), m_level(e.m_level)
				,m_enabled(e.m_enabled), m_ignore_begin_end_count(0), m_open_count(0)
				,m_instance(e.m_instance)
				,m_stream_traits(e.m_stream_traits)
			{
				sink(e.sink());
			}

			void enabled(bool enabled) { m_enabled=enabled; }
            bool enabled() const { return m_enabled; }
            void enable() { enabled(true); }
            void disable() { enabled(false); }
			basic_events(sink_type &target)
				:m_default_component(default_component<char_type>()), m_component(m_default_component)
				,m_default_topic(default_topic<char_type>()), m_topic(m_default_topic)
				,m_default_level(default_level<char_type>()), m_level(m_default_level)
				,m_enabled(true)
				,m_ignore_begin_end_count(0)
				,m_open_count(0)
			{
				reset_tags_to_default();
				sink(&target);
			}

			_Outstream &stream() { return *this; }
			const _Outstream &stream() const { return *this; }

			operator bool() const { return m_enabled; }

			template <typename Value>
			inserter operator && (const Value &v)
			{
				return inserter(*this, enabled()) && v;
			}
			template <typename Value>
			void put(const Value &v)
			{
                litwindow_logger_find_all_namespaces_operator_insert_to_stream(stream(), v);
                // This here used to be
				// stream() << v;
                // but in that case a user defined operator << will not be found if it is not
                // visible in the litwindow::logger namespace
                // So call the ::litwindow_logger... function (see below), which then
                // will cause a lookup in all accessible namespaces

                // Note: This might be a Visual Studio 2008 compiler bug, but I don't know enough
                // about the exact C++ symbol lookup rules to determine that. I would have expected
                // that symbols from the global namespace were accessible here in this function
                // even if they where defined somewhere else or somewhat later on.
                // But without this 'workaround', a user defined operator<<(ostream&, const Value&) will not be
                // found.
			}
			template <typename Value>
			inserter operator<<(const Value &v)
			{
				return operator&&<Value>(v);
			}

			_Myt &operator && (logmanipulator l)
			{
				return (*l)(*this);
				//return inserter(r, r.enabled());
			}
			_Myt &operator &&(const component_type &c)
			{
				return this->component(c);
			}
			_Myt &operator &&(const level_type &l)
			{
				return this->level(l);
			}
			_Myt &operator &&(const topic_type &t)
			{
				return this->topic(t);
			}
			void sink(sink_type *s)
			{
				m_stream_traits.set_sink(stream(), s);
			}
			sink_type *sink() const
			{
				return m_stream_traits.get_sink(stream());
			}

			_Myt &level(const level_type &l)
			{
				m_level=l; return *this;
			}
			_Myt &level(typename level_type::preset l)
			{
				return level(level_type(l));
			}
			_Myt &component(const component_type &c) 
			{
				m_component=c; return *this;
			}
			_Myt &topic(const topic_type &t)
			{
				m_topic=t; return *this;
			}
			_Myt &set_default_topic(const topic_type &t)
			{
				m_topic=m_default_topic=t; return *this;
			}
			_Myt &set_default_component(const component_type &c) 
			{
				m_component=m_default_component=c; return *this;
			}

			void continue_entry()
			{
				// ignore one 'end' and one 'begin'
				if (m_open_count)
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
						m_stream_traits.lbegin(stream());
						m_stream_traits.put_level(stream(), m_level);
						m_stream_traits.put_component(stream(), m_component);
						m_stream_traits.put_topic(stream(), m_topic);
						m_stream_traits.put_instance(stream(), m_instance);
					} else
						--m_ignore_begin_end_count;
				}
			}
			/// end an entry
			void do_end()
			{
				if (m_open_count==1) {
					if (m_ignore_begin_end_count==0) {
						m_stream_traits.lend(stream());
						reset_tags_to_default();
					} else
						--m_ignore_begin_end_count;
					m_open_count=0;
				} else if (m_open_count>0)
					--m_open_count;
			}

			void reset_tags_to_default()
			{
				m_level=m_default_level;
				m_component=m_default_component;
				m_topic=m_default_topic;
			}
			friend class inserter;
			instance_type	m_instance;
			level_type		m_default_level, m_level;
			component_type	m_default_component, m_component;
			topic_type		m_default_topic, m_topic;
		};

		template <typename _Elem, typename Streambuf>
		inline basic_events<_Elem, Streambuf> &debug(basic_events<_Elem, Streambuf> &in) { in.level(basic_level<_Elem>::debug); return in; }
		template <typename _Elem, typename Streambuf>
		inline basic_events<_Elem, Streambuf> &information(basic_events<_Elem, Streambuf> &in) { in.level(basic_level<_Elem>::information); return in; }
		template <typename _Elem, typename Streambuf>
		inline basic_events<_Elem, Streambuf> &info(basic_events<_Elem, Streambuf> &in) { return in.level(basic_level<_Elem>::info); return in; }
		template <typename _Elem, typename Streambuf>
		inline basic_events<_Elem, Streambuf> &warning(basic_events<_Elem, Streambuf> &in) { in.level(basic_level<_Elem>::warning); return in; }
		template <typename _Elem, typename Streambuf>
		inline basic_events<_Elem, Streambuf> &error(basic_events<_Elem, Streambuf> &in) { in.level(basic_level<_Elem>::error); return in; }
		template <typename _Elem, typename Streambuf>
		inline basic_events<_Elem, Streambuf> &critical(basic_events<_Elem, Streambuf> &in) { in.level(basic_level<_Elem>::critical); return in; }

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
		namespace threadsafe {
			template <typename _Events>
			struct basic_threadsafe_events
			{
				typedef _Events events_type;
				boost::thread_specific_ptr<_Events> m_evt_ptr;
				_Events m_default;
				_Events &get_default() { return m_default; }
				_Events &get() 
				{ 
					if (m_evt_ptr.get()==0) 
						m_evt_ptr.reset(new _Events(m_default)); 
					return *m_evt_ptr; 
				}
				basic_threadsafe_events()
				{
				}
				basic_threadsafe_events(const basic_threadsafe_events &evt)
					:m_default(evt.m_default)
				{
				}
				events_type &operator &&(typename events_type::logmanipulator l)
				{
					return get() && l;
				}
				template <typename Value>
				typename events_type::inserter operator &&(const Value &v)
				{
					return get() && l;
				}
				void sink(typename events_type::sink_type *s)
				{
					get().sink(s);
				}
			};
			typedef basic_threadsafe_events<events> events;
			typedef basic_threadsafe_events<wevents> wevents;
		}

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
		*
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

template <typename _Elem, typename V>
inline std::basic_ostream<_Elem> &litwindow_logger_find_all_namespaces_operator_insert_to_stream(std::basic_ostream<_Elem> &o, const V &v)
{
    return (o << v);
}

#include "logger/sink.hpp"

#ifdef _MSC_VER
#pragma once
#endif

#endif // litwindow_logger_h__151208

