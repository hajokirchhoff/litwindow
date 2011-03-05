/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: dbms.cpp,v 1.12 2008/02/26 12:10:17 Merry\Hajo Kirchhoff Exp $
*/

#include "stdafx.h"
#include <litwindow/dataadapter.h>
#include <iomanip>
#include <odbcinst.h>
#include <litwindow/check.hpp>
#include <litwindow/logging.h>
#include <io.h>
#include ".\dbms.h"
#include "statement.h"
#include "catalog.h"
#include <boost/format.hpp>
#include "dbms_ms_access.h"

using boost::str;
using boost::basic_format;

#define new DEBUG_NEW


namespace litwindow {

	namespace odbc {

		dbms_base::creator_t &dbms_base::get_dbms_register()
		{
			static creator_t dbms_register;
			return dbms_register;
		}

		sqlreturn dbms_base::construct_from_dbms_name(const tstring &name, const tstring &version, const tstring &odbc_connection_string, boost::shared_ptr<dbms_base> &ptr)
		{
			sqlreturn rc;
			try {
				creator_t::iterator i;
				for (i=get_dbms_register().begin(); i!=get_dbms_register().end() && !(i->first(name, version, odbc_connection_string)); ++i)
					;
				ptr.reset(i==get_dbms_register().end() ? new dbms_generic(odbc_connection_string) : i->second(odbc_connection_string));
				lw_log() << _T("Using '") << ptr->get_dbms_name() << _T("' strategy object for driver '") << name << _T("' version '") << version << _T("'") << endl;
			}
			catch (sqlreturn &r) {
				rc=r;
				ptr.reset();
			}
			catch (...) {
				rc=sqlreturn(_("Unknown exception while testing for driver strategy objects!"), err_logic_error);
				ptr.reset();
			}
			return rc;
		}

		tstring dbms_base::csvdelimiter()
		{
			tstring fileformat;
			TCHAR decimal_point=use_facet<numpunct<TCHAR> >(locale()).decimal_point();
			TCHAR thousands_sep=use_facet<numpunct<TCHAR> >(locale()).thousands_sep();
			if (decimal_point==_T(',') || thousands_sep==_T(','))
				fileformat=_T("Delimited(;)");
			else 
				fileformat=_T("CSVDelimited");
			return fileformat;
		}

		LWODBC_API litwindow::tstring dbms_base::construct_odbc_connection_string_from_file_name( const tstring &file, const tstring &uid/*=tstring()*/, const tstring &pwd/*=tstring()*/, bool read_only/*=false*/ ) throw()
{
			//TODO: This code should be moved into the separate dbms objects
#ifdef _WIN32
#define PATH_SEP _T('\\')
#else
#error define PATH_SEP appropriately, or better still: use boost::file once it works properly for unicode paths
#endif
			tstring rc;
			tstring extension=file.substr(max(0, file.length()-3));
			tstring file_path;
			size_t file_sep=file.rfind(PATH_SEP);
			if (file_sep!=tstring::npos)
				file_path=file.substr(0, file_sep);
			if (_tcsicmp(extension.c_str(), _T("mdb"))==0) {
				// MS-Access
				dbms_access ac(file, dbms_access::with_workgroup_if_present);
				rc=ac.get_odbc_connection_string(uid, pwd, read_only ? dbms_base::odbc_open_read_only : 0);
			} else if  (_tcsicmp(extension.c_str(), _T("xls"))==0) {
				// MS-Excel
				rc=_T("DRIVER=Microsoft Excel Driver (*.xls);UID=admin;MaxScanRows=0;MaxBufferSize=2048;ReadOnly=")+tstring((read_only?_T("1"):_T("0")))+_T(";DBQ=")+file+_T(";DefaultDir=")+file_path;
			} else if (_tcsicmp(extension.c_str(), _T("csv"))==0) {
				// comma separated
				rc=_T("DRIVER={Microsoft Text Driver (*.txt; *.csv)};Format=")+csvdelimiter()+_T(";CHARACTERSET=\"ANSI\";DefaultDir=")+file_path;
			} else if (_tcsicmp(extension.c_str(), _T("txt"))==0) {
				// text/tab separated
				rc=_T("DRIVER={Microsoft Text Driver (*.txt; *.csv)};Format=TabDelimited;CHARACTERSET=\"ANSI\";DefaultDir=")+file_path;
			}
			return rc;
		}

		sqlreturn dbms_base::register_dbms(can_handle_func_t can_handle, creator_func_t creator, void *)
		{
			creator_t &dbms_register(get_dbms_register());
			dbms_register.push_back(make_pair(can_handle, creator));
			return sqlreturn(SQL_SUCCESS);
		}

		tstring dbms_base::get_sql_column_name(const const_accessor &a) const
		{
			const static string member_prefix("m_");
			size_t pos;
			if (a.get_name().substr(0, member_prefix.size())==member_prefix)
				pos=member_prefix.size();
			else
				pos=0;
			return s2tstring(a.get_name().substr(pos));
		}

		dbms_base::dbms_base(void)
		{
			macros()[_T("NOW")]=_T("{fn now()}");
			macros()[_T("BEGIN_CREATE_VIEW")]=_T("AS");
			macros()[_T("END_CREATE_VIEW")]=_T("");
			macros()[_T("IDENTITY")]=_T("int IDENTITY");
			macros()[_T("BOOLEAN")]=_T("bit");
			macros()[_T("TRUE")]=_T("1");
			macros()[_T("FALSE")]=_T("0");
		}

		dbms_base::~dbms_base(void)
		{
		}

		tstring dbms_base::get_error_log(bool clear_error)
		{
			tstring rc(m_error_log);
			if (clear_error)
				m_error_log.clear();
			return rc;
		}

		bool_result dbms_base::call_SQLConfigDataSource(SQLHWND hwnd, const tstring &command)
		{
			bool rc;
			tstring the_command(command);
			the_command.append(1, _T(';'));
			size_t i;
			for (i=0; i<the_command.size(); ++i) {
				if (the_command[i]==_T(';'))
					the_command[i]=_T('\0');
			}
			rc=SQLConfigDataSource(hwnd, ODBC_ADD_DSN, get_driver_name().c_str(), the_command.c_str())==TRUE;
			if (!rc) {
				TCHAR message[SQL_MAX_MESSAGE_LENGTH];
				WORD text_length;
				WORD errIndex;
				DWORD err_code;
				for (errIndex=1; errIndex<=8; ++errIndex) {
					if (SQLInstallerError(errIndex, &err_code, message, sizeof(message), &text_length)==SQL_SUCCESS) {
						tostringstream o;
						o << _T("[") << (err_code) << _T("] ") << message << endl;
						m_error_log+= o.str();
					}
				}
			}
			return rc;
		}

		sqlreturn dbms_generic::get_current_sequence_value(connection *ds, const accessor &target, const tstring &sequence_name, bool expand_sequence_name_from_column) throw()
		{
			return sqlreturn(_("get_current_sequence_value is not supported"), odbc::err_not_supported_by_dbms);
		}
		sqlreturn dbms_generic::create_database(connection *ds, const tstring &database_name)
		{
			AbortOn(ds==0, "must pass a datasource to create_database");
			sqlreturn rc;
			bool isAutoCommit=ds->is_autocommit_on();
			try {
				ds->set_autocommit_on();
				tstring statement=   _T("-DROP DATABASE ") + database_name +
					_T(";-CREATE DATABASE ") + database_name +
					_T(";-USE ") + database_name;
				rc=ds->execute(statement);
			}
			catch (std::runtime_error &) {
				ds->set_autocommit_on(isAutoCommit);
				throw;
			}
			ds->set_autocommit_on(isAutoCommit);
			return rc;
		}

		sqlreturn dbms_generic::create_schema(connection *ds, const tstring &schema_name)
		{
			if (has_capability(has_create_schema)==false)
				return sqlreturn(_T("CREATE SCHEMA is not supported by this dbms"), odbc::err_not_supported_by_dbms);
			return ds->execute(_T("CREATE SCHEMA ")+schema_name);
		}
		sqlreturn dbms_generic::drop_schema(connection *ds, const tstring &schema_name)
		{
			if (has_capability(has_create_schema)==false)
				return sqlreturn(_T("DROP SCHEMA is not supported by this dbms"), odbc::err_not_supported_by_dbms);
			return ds->execute(_T("DROP SCHEMA ")+schema_name);
		}

		sqlreturn dbms_generic::use_database(connection *ds, const tstring &database_name)
		{
			return ds->execute(_T("-USE ") + database_name);
		}

		sqlreturn dbms_sql_server::create_database(odbc::connection *ds, const tstring &database_name)
		{
			try {
				return dbms_generic::create_database(ds, database_name);
			}
			catch (std::runtime_error &) {
				//tstring lastError=ds->get_last_error();
			}
			return sqlreturn(SQL_SUCCESS);
		}

		tstring dbms_base::get_macro_value(const tstring &macro_name) const
		{
			map<tstring, tstring>::const_iterator i=m_macros.find(macro_name);
			return i==m_macros.end() ? macro_name : i->second;
		}

		dbms_sql_server::dbms_sql_server(const tstring &odbcConnection)
			:dbms_generic(odbcConnection) 
		{
			macros()[_T("TIMESTAMP")]=_T("DATETIME");
            macros()[_T("UUID")]=_T("UNIQUEIDENTIFIER");
		}

		dbms_mysql::dbms_mysql(const tstring &odbcConnection)
			:dbms_generic(odbcConnection) 
		{
			macros()[_T("IDENTITY")]=_T("int AUTO_INCREMENT");
			macros()[_T("NOW")]=_T("NOW()");
			macros()[_T("VIEW")]=_T("PROCEDURE");	// mysql 4.x does not support CREATE VIEW
			macros()[_T("BEGIN_CREATE_VIEW")]=_T("()");
			macros()[_T("END_CREATE_VIEW")]=_T(";");
			//        context_t c("MySQL database");
			//        throw runtime_error("MySQL databases are not supported at this time.");
		}
		void dbms_mysql::override_driver_capabilities(connection *c)
		{
			SQLUINTEGER clear_flags=SQL_CA1_POS_UPDATE | SQL_CA1_POS_DELETE | SQL_CA1_POSITIONED_UPDATE | SQL_CA1_POSITIONED_DELETE;
#ifdef _UNICODE
			// unicode SQLBulkOperations with SQL_ADD is broken, too.
			clear_flags|=SQL_CA1_BULK_ADD;
#endif
			c->clear_cursor_attributes1(clear_flags);
		}

		//-----------------------------------------------------------------------------------------------------------//
		//-----------------------------------------------------------------------------------------------------------//
		dbms_postgres::dbms_postgres(const tstring &odbcConnection)
			:dbms_generic(odbcConnection)
		{
			macros()[_T("IDENTITY")]=_T("SERIAL");
			macros()[_T("BOOLEAN")]=_T("BOOLEAN");
			macros()[_T("TRUE")]=_T("TRUE");
			macros()[_T("FALSE")]=_T("FALSE");
			macros()[_T("DATETIME")]=_T("TIMESTAMP");
            macros()[_T("UUID")]=_T("uuid");
		}
		bool dbms_postgres::has_capability(capabilities c) const
		{
			return (c & (   has_create_database |
				has_create_schema |
				has_database |
				has_schema |
				has_user_accounts |
				has_get_current_sequence_value) ) == c;
		}
		sqlreturn dbms_postgres::get_current_sequence_value(connection *ds, const accessor &target, const tstring &sequence_name, bool expand_sequence_name_from_column) throw()
		{
			tstring seq(sequence_name);
			if (expand_sequence_name_from_column) {
				size_t colon_pos=seq.find_last_of(_T('.'));
				if (colon_pos!=tstring::npos) {
					seq[colon_pos]=_T('_');
				}
				seq+=_T("_seq");
			}
			statement s(_T("SELECT currval('")+seq + _T("')"), *ds);
			s.bind_column(1, target) && s.execute() && s.fetch();
			if (s.last_error().no_data()) {
				return sqlreturn(_("selecting currval returned no rows: ")+s.get_statement(), odbc::err_dbms_specific_error);
			}
			return s.last_error();
		}

	//-----------------------------------------------------------------------------------------------------------//
	//-----------------------------------------------------------------------------------------------------------//
		dbms_firebird::dbms_firebird(const tstring &odbcConnection)
			:dbms_generic(odbcConnection)
		{
			macros()[_T("BOOLEAN")]=_T("CHAR(1)");
			macros()[_T("TRUE")]=_T("1");
			macros()[_T("FALSE")]=_T("0");
			macros()[_T("DATETIME")]=_T("TIMESTAMP");
		}
		void dbms_firebird::override_driver_capabilities(connection *c)
		{
			SQLUINTEGER clear_flags=SQL_CA1_POS_UPDATE | SQL_CA1_POS_DELETE | SQL_CA1_POSITIONED_UPDATE | SQL_CA1_POSITIONED_DELETE;
			c->clear_cursor_attributes1(clear_flags);
		}
		//-----------------------------------------------------------------------------------------------------------//
		//-----------------------------------------------------------------------------------------------------------//

		tstring dbms_generic::get_sql_for(standard_sql_statement s) const
		{
			switch (s) {
				case sql_change_password: return _T("ALTER USER ?([in]uid) PASSWORD ?([in]newpw)");
			}
			return tstring();
		}
		sqlreturn dbms_generic::change_password(connection *ds, const tstring &oldpw, const tstring &newpw, const tstring &uid)
		{
			tstring _oldpw(oldpw), _newpw(newpw), _uid(uid);
			odbc::statement chpw(_T("ALTER USER \"")+uid+_T("\" PASSWORD ?([in]newpw)"), *ds);
			// Old password does not have to be part of the SQL statement. We're logged in anyway.
			// chpw.bind_parameter(_T("oldpw"), _oldpw);
			chpw.bind_parameter(_T("newpw"), _newpw);
			//chpw.bind_parameter(_T("uid"), _uid);
			return chpw.execute();
		}

		sqlreturn dbms_generic::create_user(connection *ds, const tstring &uid, const tstring &pwd)
		{
			return ds->execute(_T("CREATE USER \"") + uid + _T("\" PASSWORD '") + pwd + _T("'"));
		}

		sqlreturn dbms_generic::create_group(connection *ds, const tstring &gid)
		{
			return ds->execute(_T("CREATE GROUP '") + gid + _T("'"));
		}

		sqlreturn dbms_generic::add_user(connection *ds, const tstring &user, const tstring &group)
		{
			return ds->execute(_T("ADD USER '") + user + _T("' TO '") + group + _T("'"));
		}

		sqlreturn dbms_generic::drop_user(connection *ds, const tstring &user, const tstring &group)
		{
			return ds->execute(_T("DROP USER '") + user + _T("'") + ( group.size()>0 ? _T(" FROM '") + group + _T("'") : _T("") ) );
		}

		sqlreturn dbms_generic::drop_group(connection *ds, const tstring &gid)
		{
			return ds->execute(_T("DROP GROUP '") + gid + _T("'"));
		}

		SQLSMALLINT dbms_generic::sql_to_c_type(SQLSMALLINT sql_type) const
		{
			SQLSMALLINT rc=SQL_UNKNOWN_TYPE;
			switch (sql_type) {
		case SQL_CHAR:
		case SQL_VARCHAR:
		case SQL_LONGVARCHAR:   rc=SQL_C_CHAR;
			break;
		case SQL_WCHAR:
		case SQL_WVARCHAR:
		case SQL_WLONGVARCHAR:  rc=SQL_C_WCHAR;
			break;
		case SQL_DECIMAL:
		case SQL_NUMERIC:       rc=SQL_C_NUMERIC;
			break;
		case SQL_SMALLINT:      rc=SQL_C_SHORT;
			break;
		case SQL_INTEGER:       rc=SQL_C_LONG;
			break;
		case SQL_REAL:          rc=SQL_C_FLOAT;
			break;
		case SQL_FLOAT:
		case SQL_DOUBLE:        rc=SQL_C_DOUBLE;
			break;
		case SQL_BIT:           rc=SQL_C_BIT;
			break;
		case SQL_TINYINT:       rc=SQL_C_TINYINT;
			break;
		case SQL_BIGINT:        rc=SQL_C_SBIGINT;
			break;
		case SQL_BINARY:
		case SQL_VARBINARY:
		case SQL_LONGVARBINARY: rc=SQL_C_BINARY;
			break;
		case SQL_DATE:	rc=SQL_C_DATE; break;
		case SQL_TIME:	rc=SQL_C_TIME; break;
		case SQL_TIMESTAMP: rc=SQL_C_TIMESTAMP; break;
		case SQL_TYPE_DATE:     rc=SQL_C_TYPE_DATE;
			break;
		case SQL_TYPE_TIME:     rc=SQL_C_TYPE_TIME;
			break;
		case SQL_TYPE_TIMESTAMP:rc=SQL_C_TYPE_TIMESTAMP;
			break;
		case SQL_GUID:          rc=SQL_C_GUID;
			break;
			}
			return rc;
		}
		tstring dbms_generic::sql_to_create_table_name(connection *ds, SQLSMALLINT sql_type, SQLLEN length) const
		{
			SQLSMALLINT original_sql_type=sql_type;
			tstring rc;
			bool has_tried_sql_varchar=false;
			do {
				has_tried_sql_varchar=has_tried_sql_varchar || sql_type==SQL_VARCHAR;
				rc=m_type_mapper.get_type_for(ds, sql_type, length);
				if (rc.empty()) {
					// try replacement data types
					switch (sql_type) {
						case SQL_WLONGVARCHAR:	sql_type=SQL_WVARCHAR; break;
						case SQL_WVARCHAR:		sql_type=SQL_VARCHAR; break;
						case SQL_WCHAR:		sql_type=SQL_CHAR; break;

						case SQL_CHAR:			sql_type= has_tried_sql_varchar ? SQL_UNKNOWN_TYPE : SQL_VARCHAR; break;
						case SQL_LONGVARCHAR:	sql_type=SQL_VARCHAR; break;
						case SQL_TINYINT:		sql_type=SQL_SMALLINT; break;
						case SQL_SMALLINT:		sql_type=SQL_INTEGER; break;
						case SQL_INTEGER:		sql_type=SQL_DOUBLE; break;
						case SQL_FLOAT:			sql_type=SQL_REAL; break;
						case SQL_REAL:			sql_type=SQL_DOUBLE; break;
						case SQL_DOUBLE:		sql_type=SQL_DECIMAL; break;
						case SQL_BIT:			sql_type=SQL_CHAR; break;
						case SQL_TYPE_TIMESTAMP: sql_type=SQL_TIMESTAMP; break;
						case SQL_TYPE_TIME:		sql_type=SQL_TIME; break;
						case SQL_TYPE_DATE:		sql_type=SQL_DATE; break;
						case SQL_TIMESTAMP: sql_type=SQL_WVARCHAR; break;
						case SQL_TIME:		sql_type=SQL_TYPE_TIMESTAMP; break;
						case SQL_DATE:		sql_type=SQL_TYPE_TIMESTAMP; break;
						default:
							sql_type=SQL_UNKNOWN_TYPE;	// no type found
							break;
					}
					if (sql_type==SQL_UNKNOWN_TYPE)
						throw runtime_error(boost::str(boost::format("could not find CREATE TABLE data type for sql_type=%1%, dbms=%2%") % original_sql_type % t2string(get_dbms_name())).c_str());
				}
			} while (rc.empty());
#ifdef _NOT
			const TCHAR *fmt=0;
			switch (sql_type) 
			{
			case SQL_CHAR:
				fmt=_T("char");
				break;
			case SQL_VARCHAR:
				fmt=_T("varchar");
			case SQL_LONGVARCHAR:
				fmt=_T("$TEXT");
				break;
			case SQL_WCHAR:
				fmt=_T("char");
				break;
			case SQL_WVARCHAR:
				fmt=_T("varchar");
				break;
			case SQL_WLONGVARCHAR:
				fmt=_T("$TEXT");
				break;
			case SQL_DECIMAL:
			case SQL_NUMERIC:
				fmt=_T("numeric");
				break;
			case SQL_SMALLINT:
				fmt=_T("int2");
				break;
			case SQL_INTEGER:
				fmt=_T("int4");
				break;
			case SQL_REAL:
				fmt=_T("float4");
				break;
			case SQL_FLOAT:
			case SQL_DOUBLE:
				fmt=_T("float8");
				break;
			case SQL_BIT:
				fmt=_T("$BOOLEAN");
				break;
			case SQL_TINYINT:
				fmt=_T("int1");
				break;
			case SQL_BIGINT:
				fmt=_T("int8");
				break;
			case SQL_BINARY:
			case SQL_VARBINARY:
			case SQL_LONGVARBINARY:
				fmt=_T("binary");
				break;
			case SQL_TYPE_DATE:
				fmt=_T("date");
				break;
			case SQL_TYPE_TIME:
				fmt=_T("time");
				break;
			case SQL_TYPE_TIMESTAMP:
				fmt=_T("timestamp");
				break;
			case SQL_GUID:
				fmt=_T("guid");
				break;
			}
			basic_format<TCHAR> printer(fmt);
			printer.exceptions(boost::io::all_error_bits ^ boost::io::too_many_args_bit);
			return length>0 ? str(printer % length) : fmt;
#endif // _NOT
			return rc;
		}
		//-----------------------------------------------------------------------------------------------------------//
		//-----------------------------------------------------------------------------------------------------------//

		//#region SQL-Server create user statements
		//-----------------------------------------------------------------------------------------------------------//
		//-----------------------------------------------------------------------------------------------------------//

		//TODO: wxDb.Exec liefert einen Fehler zurück, auch wenn der SQL Server lediglich zusätzliche Informationen mitliefer.
		///@todo wxDb.Exec ersetzen durch eine eigene Exec Funktion, die zwischen echten Fehlern und nur zusätzlicher Information unterscheidet!
		sqlreturn dbms_sql_server::create_user(connection *ds, const tstring &uid, const tstring &)
		{
			return ds->execute(_T("-EXEC sp_grantdbaccess '") + uid + _T("'"));
		}

		sqlreturn dbms_sql_server::create_group(connection *ds, const tstring &gid)
		{
			return ds->execute(_T("-EXEC sp_addrole '") + gid + _T("'"));
		}

		sqlreturn dbms_sql_server::add_user(connection *ds, const tstring &user, const tstring &group)
		{
			return ds->execute(_T("-EXEC sp_addrolemember '") + group + _T("', '") + user + _T("'"));
		}
		sqlreturn dbms_sql_server::get_current_sequence_value(connection *ds, const accessor &target, const tstring &sequence_name, bool expand_sequence_name_from_column) throw()
		{
			statement s(_("SELECT @@IDENTITY"), *ds);
			s.bind_column(1, target) && s.execute() && s.fetch();
			if (s.last_error().no_data()) {
				return sqlreturn(_("selecting @@IDENTITY returned no rows: ")+s.get_statement(), odbc::err_dbms_specific_error);
			}
			return s.last_error();
		}

		sqlreturn dbms_sql_server::drop_user(connection *ds, const tstring &user, const tstring &group)
		{
			return ds->execute(group.size()==0 ? 
				_T("-EXEC sp_revokedbaccess '") + user + _T("'") :
			_T("-EXEC sp_droprolemember '") + group + _T("', '") + user + _T("'"));
		}

		sqlreturn dbms_sql_server::drop_group(connection *ds, const tstring &gid)
		{
			return ds->execute(_T("-EXEC sp_droprole '") + gid + _T("'"));
		}
		//#endregion

		bool dbms_sql_server::can_handle_(const tstring &name, const tstring &version, const tstring &odbc_connection_string)
		{
			return name==_T("Microsoft SQL Server") || name==_T("SQL Server");
		}
		bool dbms_mysql::can_handle_(const tstring &name, const tstring &version, const tstring &odbc_connection_string)
		{
			if (name==_T("MySQL") || name==_T("MySQL ODBC 3.51 Driver")) {
				bool is_broken=false;
#ifdef _UNICODE
				if (version==_T("4.1.9-nt"))
					is_broken=true;
#endif
				if (is_broken) {
					throw sqlreturn(
#ifdef _UNICODE
						tstring(_("Unicode ")) +
#endif
						tstring(_("Version ")) + version + _T(" of the MySQL ODBC driver is buggy and is not supported. Please update your driver to the lastest version."), err_not_supported_by_dbms);
				}
				return !is_broken;
			}
			return false;
		}
		bool dbms_postgres::can_handle_(const tstring &name, const tstring &version, const tstring &odbc_connection_string)
		{
			return name==_T("PostgreSQL");
		}

		litwindow::odbc::sqlreturn dbms_postgres::add_user( connection *ds, const tstring &user, const tstring &group )
		{
			return ds->execute(_T("GRANT \"")+group+_T("\" TO \"")+user+_T("\""));
		}

		litwindow::odbc::sqlreturn dbms_postgres::drop_user( connection *ds, const tstring &uid, const tstring &group )
		{
			return group.empty()
				? ds->execute(_T("DROP \"")+uid+_T("\""))
				: ds->execute(_T("REVOKE \"")+group+_T("\" FROM \"")+uid+_T("\""));
		}
		bool dbms_firebird::can_handle_(const tstring &name, const tstring &version, const tstring &)
		{
			return name==_T("Firebird 1.5");
		}
		litwindow::odbc::sqlreturn dbms_postgres::create_group(connection *ds, const tstring &gid)
		{
			return ds->execute(_T("CREATE GROUP \"")+gid+_T("\""));
		}
		litwindow::odbc::sqlreturn dbms_postgres::drop_group(connection *ds, const tstring &gid)
		{
			return ds->execute(_T("DROP GROUP \"")+gid+_T("\""));
		}

		namespace {
			dbms_base::do_register	sql_server(dbms_sql_server::can_handle_, dbms_sql_server::construct);
			dbms_base::do_register	mysql(dbms_mysql::can_handle_, dbms_mysql::construct);
			dbms_base::do_register	postgres(dbms_postgres::can_handle_, dbms_postgres::construct);
			dbms_base::do_register	firebird(dbms_firebird::can_handle_, dbms_firebird::construct);
		};
		
		litwindow::tstring dbms_type_mapper::get_type_for( connection *ds, SQLSMALLINT data_type, SQLLEN length ) const
		{
			if (m_type_info.empty()) {
				odbc::catalog c(*ds);
				sql_type_info_result_set rs;
				c.get_type_information(rs);
				while (c.fetch()) {
					m_type_info.push_back(rs);
				}
			}
			size_t i;
			for (i=0; i<m_type_info.size() && m_type_info[i].SQL_DATA_TYPE!=data_type; ++i)
				;
			return i<m_type_info.size() ? m_type_info[i].TYPE_NAME : tstring();
		}
	};

};