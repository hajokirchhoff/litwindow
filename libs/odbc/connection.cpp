/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: connection.cpp,v 1.11 2007/07/20 10:16:23 Hajo Kirchhoff Exp $
*/

#include "stdafx.h"

#include <sqlext.h>
#include <malloc.h>

#include <boost/thread/mutex.hpp>

#include "litwindow/logging.h"
#include "litwindow/check.hpp"
#include "litwindow/odbc/connection.h"
#include "litwindow/odbc/binder.h"
#include "litwindow/odbc/statement.h"

#define new DEBUG_NEW

#pragma warning(disable: 4312 4267)

namespace litwindow {

	namespace odbc {

	//-----------------------------------------------------------------------------------------------------------//
	//-----------------------------------------------------------------------------------------------------------//

	connection::dbversion &connection::dbversion::operator=(std::wstring &new_version)
	{
		std::wistringstream stream(new_version);
		stream >> major;
		if (stream.peek() == '.') {
			stream.ignore();
			stream >> minor;
		}

		return *this;
	}

	//-----------------------------------------------------------------------------------------------------------//
	//-----------------------------------------------------------------------------------------------------------//

	transaction::~transaction()
	{
		if (m_transaction_is_open) {
			if (std::uncaught_exception())
				m_connection.rollback_transaction();
			else
				m_connection.commit_transaction();
		}
	}

	sqlreturn transaction::commit()
	{
		sqlreturn rc=m_transaction_is_open ? m_connection.commit_transaction() : sqlreturn(SQL_SUCCESS);
		m_transaction_is_open=false;
		return rc;
	}

	sqlreturn transaction::rollback()
	{
		sqlreturn rc=m_transaction_is_open ? m_connection.rollback_transaction() : sqlreturn(SQL_SUCCESS);
		m_transaction_is_open=false;
		return rc;
	}

	bool extract_dsn_uid_pwd(const tstring &odbc_connection_string, tstring &dsn, tstring &uid, tstring &pwd, tstring &remaining_connection_string)
	{
		dsn.clear();
		uid.clear();
		pwd.clear();
		enum {
			keyword,
			value,
			quoted_value,
			end_of_value,
			end_of_string
		} state=keyword;
		bool unknown_keyword=false;
		const TCHAR *p;
		const TCHAR *begin_keyword;
		const TCHAR *end_keyword=nullptr;
		const TCHAR *begin_value=nullptr;
		const TCHAR *end_value=nullptr;
		p=begin_keyword=odbc_connection_string.c_str();
		remaining_connection_string.clear();
		if (!*p)
			return false;
		do {
			switch (state) {
			case keyword:
				{
					if (*p==_T('=')) {
						state=value;
						begin_value=p+1;
						end_keyword=p;
					}
				} break;
			case quoted_value:
				{
					if (*p==_T('"')) {
						state=end_of_value;
						end_value=p;
					}
				} break;
			case value:
				{
					if (*p==_T('"')) {
						state=quoted_value;
						begin_value=p+1;
					}  else if (*p==_T(';') || !*p) {
						end_value=p;
						--p;	// put back ';'
						state=end_of_value;
					}
				} break;
			case end_of_value:
				{
					if (*p==';' || !*p) {
						TCHAR the_keyword[32];
						memcpy(the_keyword, begin_keyword, min(sizeof(the_keyword), (end_keyword-begin_keyword)*sizeof(*begin_keyword)));
						use_facet<ctype<TCHAR> >(locale()).toupper(the_keyword, the_keyword+sizeof(the_keyword)/sizeof(*the_keyword));
						tstring k(the_keyword, end_keyword-begin_keyword);
						tstring *assign_value=nullptr;
						if (k==_T("DSN"))
							assign_value=&dsn;
						else if (k==_T("UID"))
							assign_value=&uid;
						else if (k==_T("PWD"))
							assign_value=&pwd;
						else {
							remaining_connection_string+=tstring(begin_keyword, end_value-begin_keyword)+_T(';');
							unknown_keyword=true;
						}
						if (assign_value) {
							*assign_value=tstring(begin_value, end_value-begin_value);
						}
						state=keyword;
						begin_keyword=p+1;
					}
				} break;
			}
		} while (*p++);
		return state==keyword;
	}

	int connection::sm_unit_test_mode = 0;
	/* \internal set the test mode for unittests.
	-   1 - do not execute script commands. Place them in sm_unit_test_result instead.
	*/
	int &connection::unit_test_mode_()
	{
		return sm_unit_test_mode;
	}
	tstring connection::sm_unit_test_result;
	tstring &connection::unit_test_result_()
	{
		return sm_unit_test_result;
	}

	namespace implementation
	{
		class pool:public connection::pool_imp_base
		{
		protected:
			using entry = connection::shared_ptr;
			using entries_t = vector<pair<tstring, entry> >;
			entries_t m_entries;
			using mutex = boost::mutex;
			using scoped_lock = mutex::scoped_lock;
			mutex m_lock;
		public:
			connection::shared_ptr open(const tstring &name = tstring()) override
			{
				scoped_lock l(m_lock);
				connection::shared_ptr rc = do_find_entry(name);
				if (!rc->is_open())
					rc->open();
				return rc;
			}

			connection::shared_ptr get(const tstring &name) override
			{
				entry e=find_entry(name);
				return e;
			}
			void set(const tstring &dsn, const tstring &uid, const tstring &pwd, const tstring &name) override
			{
				set(_T("DSN=")+dsn+_T(";UID=")+uid+_T(";PWD=")+pwd, name);
			}
			void set(const tstring &connection_string, const tstring &name) override
			{
				entry e=find_entry(name);
				e->set_connection_string(connection_string);
			}
			void put(const tstring &name, connection::shared_ptr connection) override
			{
				entry e=find_entry(name);
				e=connection;
			}
			entry find_entry(const tstring &name)
			{
				scoped_lock l(m_lock);
				return do_find_entry(name);
			}

			entry do_find_entry(const tstring &name)
			{
				entries_t::iterator i;
				for (i=m_entries.begin(); i!=m_entries.end() && i->first!=name; ++i)
					;
				if (i==m_entries.end()) {
					m_entries.emplace_back(name, entry());
					i=m_entries.end()-1;
				}
				if (i->second.get() == nullptr) {
					i->second.reset(new connection);
					if (name.empty() == false && m_entries.empty() == false)
						i->second->set_connection_string(m_entries.front().second->get_connection_string());
				}
				return i->second;
			}
			void close_all() override
			{
				scoped_lock l(m_lock);
				entries_t::iterator i;
				for (i=m_entries.begin(); i!=m_entries.end(); ++i) {
					i->second.reset();
				}
			}
			///@hack: This is a workaround for a bug (seemingly) in the MS Driver Manager
			/// Calling 'close' when pool is being destructed in the DLL _CRT_INIT exit sequence
			/// causes an access violation
			/// The workaround catches those access violations and hopes for the best, i.e.
			/// that the driver manager will automatically clean up all connections, even if
			/// 'close' failed.
			/// --But unfortunately it doesn't seem to. MS-Jet leaves the *.ldb file around!
			~pool()
			{
				entries_t::iterator i;
				for (i=m_entries.begin(); i!=m_entries.end(); ++i) {
					if (i->second) {
						try {
							i->second.reset();
						}
						catch (...) {
							// silently eat up errors
							static bool err_shown=false;
							if (err_shown==false) {
								err_shown=true;
								//#ifdef _DEBUG
#ifdef _WIN32
								//-----------------------------------------------------------------------------------------------------------//
								/* 
								NOTE:   Under certain situations the connection::pool() cannot automatically close connections that
								are still open when your application terminates.
								So far this behaviour has been observed under the following conditions:
								+ You are using the DLL version of the lwodbc library.
								+ You are using the DLL version of the C++ runtime library.
								+ You are compiling and using the Debug version of your application and both libraries.
								+ You exit your application while at least one ODBC connection is still open.
								What happens is: If the connection::pool() tries to close a connection automatically from the
								onexit cleanup section of the DLL, the ODBC Driver Manager or the ODBC Driver itself causes
								an access violation. This does not happen if you use the static version of the lwodbc library,
								since the cleanup code is then called from the application itself, not from the DLL 'DETACH_PROCESS'
								handler.
								So far this has been observed only with the Debug versions of the runtime library. Release
								seems to work fine.
								But to be on the save side:

								Please call    litwindow::odbc::connection::pool().close_all()    before exiting your application!
								*/
								DebugBreak();
								//-----------------------------------------------------------------------------------------------------------//
#endif
								//#endif
							}
						}
					}
				}
			}
		};
	};

	connection::pool_imp_base &connection::pool()
	{
		static implementation::pool g_pool;
		return g_pool;
	}

	const sqlreturn &connection::end_transaction(SQLSMALLINT completion_type)
	{
		litwindow::Precondition(m_nested_transactions>0, "calls to begin_transaction and end_transaction are not balanced");
		if (--m_nested_transactions==0) {
			m_last_error=SQLEndTran(SQL_HANDLE_DBC, handle(), completion_type);
			if (m_autocommit_state)	// if autocommit was on before this transaction started, set it back on
				set_autocommit_on();
		} else
			m_last_error.clear();
		return m_last_error;
	}

	const sqlreturn &connection::begin_transaction()
	{
		if (++m_nested_transactions==1)
			m_last_error=set_autocommit_off();
		else
			m_last_error.clear();
		return m_last_error;
	}

	connection::connection()
		:m_dbms(new dbms_generic(tstring())),m_throw_on_error_default(false)
	{
		init(get_default_environment());
	}

	connection::connection(dbms_base *dbms_to_use, environment env)
		:m_dbms(dbms_to_use),m_throw_on_error_default(false)
	{
		init(env);
	}

	void connection::init(environment env)
	{
		m_log=true;
		m_is_connected=false;
		m_env=env;
		m_nested_transactions=0;
		m_autocommit_state=true;
		alloc_handle(env);
		//sqlreturn_auto_set_diagnostics rc;
		//rc.set_handles(SQL_HANDLE_ENV, env->handle());
		//rc=SQLAllocHandle(SQL_HANDLE_DBC, env->handle(), &m_handle);
		//touch();   // set the time the handle was accessed
		//rc.assert_success();
		//m_last_error.set_handles(SQL_HANDLE_DBC, handle());
		// tell ODBC to use the cursor library if neccessary
		set_cursor_implementation(connection::g_default_cursor_implementation);
	}

	const sqlreturn &connection::reset()
	{
		if (m_is_connected)
			close();
		free_handle();
		init(m_env);
		return m_last_error;
	}

	connection::cursor_implementation_enum connection::g_default_cursor_implementation=connection::use_driver_cursors;

	const sqlreturn &connection::set_cursor_implementation(cursor_implementation_enum crs)
	{
		return set_attribute(SQL_ATTR_ODBC_CURSORS, crs);
	}

	void connection::set_default_cursor_implementation(cursor_implementation_enum default_crs)
	{
		g_default_cursor_implementation=default_crs;
	}

	connection::~connection()
	{
		if (m_is_connected)
			close();
		free_handle();
	}

	const sqlreturn &connection::free_handle()
	{
		m_last_error.set_handles(0, nullptr);
		SQLHANDLE h=handle();
		m_handle=SQL_NULL_HANDLE;
		return m_last_error=SQLFreeHandle(SQL_HANDLE_DBC, h);
	}

	const sqlreturn &connection::alloc_handle(environment env)
	{
		m_last_error.set_handles(SQL_HANDLE_ENV, env->handle());
		m_handle=SQL_NULL_HANDLE;
		m_last_error=SQLAllocHandle(SQL_HANDLE_DBC, env->handle(), &m_handle);
		touch();
		m_last_error.set_handles(m_handle==SQL_NULL_HANDLE ? 0 : SQL_HANDLE_DBC, handle());
		return m_last_error;
	}

	const sqlreturn &connection::set_connection_string(const tstring &connection_string)
	{
		if (extract_dsn_uid_pwd(connection_string, m_dsn, m_uid, m_pwd, m_remaining_connection_string)==false) {
			m_last_error=sqlreturn(_("Malformed connection string"), err_logic_error);
		} else {
			m_last_error.clear();
		}
		return m_last_error;
	}

	bool connection::set_dsn(const tstring &dsn)
	{
		m_remaining_connection_string.clear();
		m_dsn=dsn;
		return true;
	}

	tstring connection::get_connection_string() const
	{
		tstring rc;
		const tstring *join[]={&m_dsn, &m_uid, &m_pwd, &m_remaining_connection_string};
		const TCHAR *keyword[]={_T("DSN="), _T("UID="), _T("PWD="), _T("")};
		size_t i;
		for (i=0; i<sizeof(join)/sizeof(*join); ++i) {
			if (join[i]->length()) {
				if (join[i]->empty()==false) {
					if (rc.length()) rc+=_T(';');
					rc+=keyword[i];
					rc+=*join[i];
				}
			}
		}
		return rc;
	}

	const sqlreturn &connection::open(const litwindow::tstring &dsn, const litwindow::tstring &uid, const litwindow::tstring &pwd)
	{
		bool is_connection_string=dsn.find_first_of(_T(";="))!=tstring::npos;
		if (is_connection_string)
			set_connection_string(dsn);
		else
			set_dsn(dsn);
		set_uid(uid);
		set_pwd(pwd);
		(is_open()==false || close().success()) && open();
		return m_last_error;
	}

	const sqlreturn &connection::open(SQLHWND hwnd, SQLUSMALLINT completion)
	{
		if (is_open())
			return m_last_error=SQL_SUCCESS;
		if (hwnd!=nullptr && completion==SQL_DRIVER_NOPROMPT)
			completion=SQL_DRIVER_COMPLETE;
		if (is_open_via_SQLConnect() && completion==SQL_DRIVER_NOPROMPT) {
			// SQLConnect was defined before 'const' came around. Thus it has SQLTCHAR* as parameters instead of
			// const SQLTCHAR* as should be. Worse yet, SQLCHAR is defined as unsigned char, whereas c_str() returns
			// char*. So simply cast the parameters to work around this limitation.
			m_last_error=SQLConnect(handle(), (SQLTCHAR*)m_dsn.c_str(), SQL_NTS, 
				(SQLTCHAR*)m_uid.c_str(), SQL_NTS, 
				(SQLTCHAR*)m_pwd.c_str(), SQL_NTS);
			m_out_connection_string=get_connection_string();
		} else {
			SQLTCHAR outbuffer[2048];
			SQLSMALLINT outbuffer_length;
			tstring connection_string(get_connection_string());
			m_last_error=SQLDriverConnect(handle(), hwnd, 
				(SQLTCHAR*)connection_string.c_str(), (SQLSMALLINT)connection_string.length(),
				outbuffer, sizeof(outbuffer), &outbuffer_length, completion);
			m_out_connection_string=(TCHAR*)outbuffer;
		}
		if (m_last_error) {
			m_last_error=get_driver_capabilities_and_parameters();
			if (m_last_error.fail())
				close();
		}
		m_is_connected=m_last_error.success();
		if (!m_is_connected) {
			//SQLDisconnect(handle());
		}
		return m_last_error;
	}

	const sqlreturn &connection::open(const litwindow::tstring &connection_string, SQLHWND hwnd, SQLUSMALLINT completion)
	{
		set_connection_string(connection_string) && (is_open()==false || close().success()) && open(hwnd, completion);
		return m_last_error;
	}

	bool connection::require(SQLUSMALLINT what, SQLUINTEGER expected, SQLUINTEGER bitmask)
	{
		SQLUINTEGER value=0;
		bool rc=get_info(what, value).success() && (value & bitmask)==expected;
		if (!rc && is_log()) {
			lw_log() << _T("odbc::connection::require ") << what << _T(", ") << expected << _T(", ") << bitmask << _T(" returns false.") << endl;
		}
		return rc;
	}

#define REQUIRE_BITS_MASK(a,b,c,text1,text2) { bool result=require((SQLSMALLINT)a, (SQLUINTEGER)b, (SQLUINTEGER)c); if (!result) { lw_err() << _T("requires ") << text1 << _T('=') << text2 << endl; } rc=rc&&result; }
#define REQUIRE_BITS(a,b) REQUIRE_BITS_MASK(a,b,b,#a,#b)
#define REQUIRE_VALUE(a,b) REQUIRE_BITS_MASK(a,b,~0,#a,#b)

	const sqlreturn &connection::get_driver_capabilities_and_parameters()
	{
		m_last_error.clear();
		// first get the drivers name so we can set the DBMS strategy
		get_info(SQL_SERVER_NAME, m_dbms_name);
		// is broken in MySQL!!! --> 	get_info(SQL_DATABASE_NAME, m_dbms_name);
		// is broken in MySQL!!! -->	get_info(SQL_USER_NAME, m_dbms_name);
		// is broken in MySQL!!! -->	get_info(SQL_DATA_SOURCE_NAME, m_dbms_name);
		get_info(SQL_DRIVER_NAME, m_dbms_name);
		get_info(SQL_DBMS_NAME, m_dbms_name).log_errors();
		get_info(SQL_DBMS_VER, m_dbms_ver).log_errors();

		std::wstring ver_string;
		get_info(SQL_DRIVER_ODBC_VER, ver_string);
		m_dbms_odbc_ver = ver_string;
		get_info(SQL_DRIVER_VER, ver_string);
		m_dbms_driver_ver = ver_string;

		//if (m_dbms->get_dbms_name()!=m_dbms_name)
		m_last_error=dbms_base::construct_from_dbms_name(m_dbms_name, m_dbms_ver, m_out_connection_string, m_dbms);
		if (m_last_error.fail())
			return m_last_error;

		m_identifier_quote_char.clear();

		//REQUIRE_BITS(SQL_SQL_CONFORMANCE, SQL_SC_SQL92_ENTRY);

		//	bool rc=true;

		//REQUIRE_VALUE(SQL_NON_NULLABLE_COLUMNS, SQL_NNC_NON_NULL);

		// check if we can call SQLGetData in any order on any column
		//REQUIRE_BITS(SQL_GETDATA_EXTENSIONS, SQL_GD_ANY_COLUMN | SQL_GD_ANY_ORDER);

		//REQUIRE_BITS(SQL_SCHEMA_USAGE, SQL_SU_DML_STATEMENTS | SQL_SU_TABLE_DEFINITION | SQL_SU_PRIVILEGE_DEFINITION);

		get_info(SQL_IDENTIFIER_QUOTE_CHAR, m_identifier_quote_char).log_errors();
		get_info(SQL_BOOKMARK_PERSISTENCE, m_bookmark_persistence);

		//TODO: Check max_concurrent and max_driver values when new statements are created
		get_info(SQL_MAX_CONCURRENT_ACTIVITIES, m_max_concurrent_activities);
		get_info(SQL_MAX_DRIVER_CONNECTIONS, m_max_driver_connections);

		get_info(SQL_SCROLL_CONCURRENCY, m_scroll_concurrency);

		get_info(SQL_POS_OPERATIONS, m_pos_operations);
		get_info(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1, m_forward_only_cursor_attributes1);
		get_info(SQL_STATIC_CURSOR_ATTRIBUTES1, m_static_cursor_attributes1);
		get_info(SQL_DYNAMIC_CURSOR_ATTRIBUTES1, m_dynamic_cursor_attributes1);
		get_info(SQL_KEYSET_CURSOR_ATTRIBUTES1, m_keyset_cursor_attributes1);

		// allow dbms strategy object to override some attributes
		m_dbms->override_driver_capabilities(this);
#ifdef _DEBUG
		lw_log() << _T("Capabilities for ") << m_dbms_name << endl;
		lw_log() << capabilities_as_string();
#endif
		// Copy macros from dbms to connection
		const dbms_base *const_dbms = m_dbms.get();
		for (const auto & it : const_dbms->macros())
			set_macro_value(L"$$" + it.first, it.second);

		return m_last_error;
	}

	void connection::clear_cursor_attributes1(SQLUINTEGER clear_flags)
	{
		m_dynamic_cursor_attributes1 &= ~clear_flags;
		m_static_cursor_attributes1 &= ~clear_flags;
		m_forward_only_cursor_attributes1 &= ~clear_flags;
		m_keyset_cursor_attributes1 &= ~clear_flags;
	}

	namespace {
		struct {
			SQLSMALLINT option;
			SQLUINTEGER bitmask;
			const TCHAR *name;
		} supports_bitmasks[]={
			{ SQL_SCROLL_OPTIONS, SQL_SO_FORWARD_ONLY, _T("forward only cursors") },
			{ SQL_SCROLL_OPTIONS, SQL_SO_STATIC, _T("static cursors") },
			{ SQL_SCROLL_OPTIONS, SQL_SO_KEYSET_DRIVEN, _T("keyset driven cursors") },
			{ SQL_SCROLL_OPTIONS, SQL_SO_DYNAMIC, _T("dynamic cursors") },
			{ SQL_SCROLL_OPTIONS, SQL_SO_MIXED, _T("keyset driven mixed cursors") }
		};
		struct {
			SQLSMALLINT request;
			const TCHAR *name;
		} get_info_request[]= {
			{ SQL_CURSOR_COMMIT_BEHAVIOR, _T("Cursor Commit Behavior") },
			{ SQL_CURSOR_ROLLBACK_BEHAVIOR, _T("Cursor Rollback Behavior") },
			{ SQL_CURSOR_SENSITIVITY, _T("Cursor Sensitivity") },
			{ SQL_MAX_CONCURRENT_ACTIVITIES, _T("Maximum concurrent activities") },
			{ SQL_MAX_DRIVER_CONNECTIONS, _T("Maximum driver connections") }
		};
	};

	SQLUINTEGER connection::get_cursor_attributes1(SQLSMALLINT ctype) const
	{
		switch (ctype) {
		case SQL_CURSOR_FORWARD_ONLY : return m_forward_only_cursor_attributes1;
		case SQL_CURSOR_STATIC: return m_static_cursor_attributes1;
		case SQL_CURSOR_DYNAMIC: return m_dynamic_cursor_attributes1;
		case SQL_CURSOR_KEYSET_DRIVEN: return m_keyset_cursor_attributes1;
		}
		return 0;
	}

	tstring connection::capabilities_as_string()
	{
		tstringstream str;
		size_t i;
		SQLUINTEGER value;
		for (i=0; i<sizeof(supports_bitmasks)/sizeof(*supports_bitmasks); ++i) {
			sqlreturn err=get_info(supports_bitmasks[i].option, value);
			str << supports_bitmasks[i].name << _T(": ") << (err.ok() ?   ((value & supports_bitmasks[i].bitmask)!=0 ? _T("YES") : _T("no")) : err.as_string()) << endl;
		}
		for (i=0; i<sizeof(get_info_request)/sizeof(*get_info_request); ++i) {
			sqlreturn err=get_info(get_info_request[i].request, value);
			str << get_info_request[i].name << _T("= ") << value << endl;
		}
		return str.str();
	}

	sqlreturn connection::close()
	{
		if (m_is_connected) {
			m_is_connected=false;   // set it to false in case SQLDisconnect throws or crashes, see \hack above in implementation::pool::~pool
			m_last_error=SQLDisconnect(handle());
			m_is_connected=!m_last_error.success();
		} else
			m_last_error.clear();
		return m_last_error;
	}

	const sqlreturn &connection::set_attribute(SQLINTEGER attribute, SQLUINTEGER value)
	{
		m_last_error=SQLSetConnectAttr(handle(), attribute, (SQLPOINTER)value, SQL_IS_UINTEGER);
		return m_last_error;
	}
	const sqlreturn &connection::set_attribute(SQLINTEGER attribute, const tstring &value)
	{
		m_last_error=SQLSetConnectAttr(handle(), attribute, (SQLPOINTER)const_cast<TCHAR*>(value.c_str()), SQL_NTS);
		return m_last_error;
	}

	sqlreturn connection::get_attribute(SQLINTEGER attribute, SQLUINTEGER &value)
	{
		m_last_error=SQLGetConnectAttr(handle(), attribute, &value, SQL_IS_UINTEGER, nullptr);
		return m_last_error;
	}

	sqlreturn connection::get_attribute(SQLINTEGER attribute, tstring &value)
	{
		TCHAR buffer[512];
		TCHAR *rc_buffer=buffer;
		SQLINTEGER length=sizeof(buffer);
		m_last_error=SQLGetConnectAttr(handle(), attribute, buffer, sizeof(buffer), &length);
		if (m_last_error==SQL_SUCCESS_WITH_INFO && m_last_error.is_state(_T("01004"))) {
			rc_buffer=(TCHAR*)_alloca(length+sizeof(TCHAR));
			m_last_error=SQLGetConnectAttr(handle(), attribute, rc_buffer, length+sizeof(TCHAR), &length);
		}
		if (m_last_error.success()) {
			if (length==SQL_NTS)
				value=rc_buffer;
			else
				value=tstring(rc_buffer, length/sizeof(TCHAR));
		}
		return m_last_error;
	}

	sqlreturn connection::get_info(SQLUSMALLINT info, SQLUINTEGER &value)
	{
		SQLSMALLINT length;
		value=0;
		m_last_error=SQLGetInfo(handle(), info, &value, sizeof(SQLUINTEGER), &length);
#if !defined(_WIN32)
#pragma message("This code assumes LSB byte order!")
#endif
		return m_last_error;
	}

	sqlreturn connection::get_info(SQLUSMALLINT info, tstring &value)
	{
		TCHAR buffer[512];
		TCHAR *rc_buffer=buffer;
		SQLSMALLINT length;
		m_last_error=SQLGetInfo(handle(), info, buffer, sizeof(buffer), &length);
		if (m_last_error==SQL_SUCCESS_WITH_INFO && m_last_error.is_state(_T("01004"))) {
			rc_buffer=(TCHAR*)_alloca(length+sizeof(TCHAR));
			m_last_error=SQLGetInfo(handle(), info, rc_buffer, length+sizeof(TCHAR), &length);
		}
		if (m_last_error.success()) {
			if (length==SQL_NTS)
				value=rc_buffer;
			else
				value=tstring(rc_buffer, length/sizeof(TCHAR));
		}
		return m_last_error;
	}

	const sqlreturn LWODBC_API & connection::open_file( const litwindow::tstring &file, const litwindow::tstring &uid, const litwindow::tstring &pwd, bool read_only/*=false*/, const litwindow::tstring &dsn_addition/*=litwindow::tstring()*/, const litwindow::tstring &file_type/*=litwindow::tstring()*/ ) throw()
	{
		tstring odbc_connection_string=dbms_base::construct_odbc_connection_string_from_file_name(file, uid, pwd, read_only, file_type);
		if (dsn_addition.empty()==false)
			odbc_connection_string+=_T(';')+dsn_addition;
		set_dsn(wstring());
		return open(odbc_connection_string);
	}

	void connection::set_macro_value( const tstring &name, const tstring &value )
	{
		litwindow::Precondition(name.substr(0, 2)==_T("$$"), "macro names must begin with $$");
		m_macros[name]=value;
	}

    const sqlreturn & connection::add_or_alter_column_for( prop_t a, const tstring &column_name, const tstring &table_name )
    {
        data_type_info info;
        m_last_error=data_type_lookup().get(a, info);
        if (m_last_error.ok()) {
            SQLSMALLINT sql_type=info.m_sql_type;
            tstring sql_type_name=sql_to_create_table_name(sql_type, info.m_column_size);
            statement stmt(*this);
            stmt << _T("ALTER TABLE ") << table_name.c_str() << _T(" COLUMN ") << binder::make_column_name(column_name).c_str() << _T(" ") << sql_type << _T(";");
            if (!stmt.execute()) {
                stmt.clear();
                stmt << _T("ALTER TABLE ") << table_name.c_str() << _T(" ADD COLUMN ") << binder::make_column_name(column_name).c_str() << _T(" ") << sql_type << _T(";");
                stmt.execute();
            }
            m_last_error=stmt.last_error();
        }
        return m_last_error;
    }

    const sqlreturn & connection::create_table_for( const_aggregate ag, const tstring &table_name/*=tstring()*/, const tstring &primarykey_name/*=tstring()*/ )
    {
        statement stmt(*this);
        stmt << _T("CREATE TABLE ") << table_name.c_str() << _T(" ()");
        stmt.execute();
        return m_last_error;
    }
    //-----------------------------------------------------------------------------------------------------------//
	//-----------------------------------------------------------------------------------------------------------//
	environment_imp::environment_imp()
	{
		sqlreturn_auto_set_diagnostics rc;
		rc=SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_handle);
		rc.assert_success();
		rc.set_handles(SQL_HANDLE_ENV, m_handle);
		SQLINTEGER version=SQL_OV_ODBC3;
		rc=SQLSetEnvAttr(handle(), SQL_ATTR_ODBC_VERSION, (SQLPOINTER)version, SQL_IS_INTEGER);
		rc.assert_success();
	}

	environment_imp::~environment_imp()
	{
		SQLFreeHandle(SQL_HANDLE_ENV, handle());
	}

	bool LWODBC_API CheckSQLReturn(SQLRETURN rc)
	{
		if (rc==SQL_ERROR) {
			throw std::runtime_error("SQL_ERROR");
		}
		return true;
	}

	environment get_default_environment()
	{
		static environment instance(new environment_imp);
		return instance;
	}

	};

};
