/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: dbms.h,v 1.10 2007/06/08 08:11:38 Hajo Kirchhoff Exp $
*/

#ifndef __LWODBC_DBMS_H
#define __LWODBC_DBMS_H

#pragma once

#include "lwodbc.h"
#include <litwindow/result.hpp>
#include <litwindow/dataadapter.h>
#include <map>
#include <boost/smart_ptr.hpp>
#include <boost/function.hpp>
#include <map>
#pragma warning(push, 4)

// now make intellisense work again by undefining LWODBC_API for intellisense
#ifdef INTELLISENSE_DOES_NOT_WORK_PSEUDO_DEFINE
#undef LWODBC_API
#define LWODBC_API
#endif

namespace litwindow {

	class accessor;

	namespace odbc {

		using namespace std;

		class connection;

		/// the type mapper maps SQL data types to their DBMS type names
		class dbms_type_mapper
		{
		protected:
			mutable vector<sql_type_info_result_set> m_type_info; // lazy evaluation, thus -> mutable
		public:
			tstring get_type_for(connection *ds, SQLSMALLINT data_type, SQLLEN length) const;
		};

		/** Base DBMS strategy class */
		class dbms_base
		{
		public:
			/// List of DBMS capabilities.
			/// This is a list of possible DBMS capabilities that may or may not be supported by the current
			/// connection. The connection tries to fail gracefully if an unsupported action is attempted.
			/// But for some capabilities it will be neccessary to test availability before attempting an action.
			/// Use with 'supports'
			enum capabilities {
				has_user_accounts = 1,	///< has different user accounts and login rights
				has_schema = 2,                     ///< has SCHEMA support
				has_create_schema = 4,     ///< can CREATE SCHEMA
				has_database = 8,                  ///< supports using different databases - USE DATABASE
				has_create_database = 0x10,         ///< supports creating a database
				has_get_current_sequence_value = 0x20,		///< has get_current_sequence_value
				last__cap_last
			};
			LWODBC_API dbms_base();
			LWODBC_API virtual ~dbms_base();

			/// find a dbms strategy for the driver name and construct a new dbms_base object on the heap
			static LWODBC_API sqlreturn construct_from_dbms_name(const tstring &name, const tstring &version, const tstring &odbc_connection_string, boost::shared_ptr<dbms_base> &ptr);

			/// take a file name and build an odbc connection string for it
			static LWODBC_API litwindow::tstring construct_odbc_connection_string_from_file_name(const tstring &file, const tstring &uid=tstring(), const tstring &pwd=tstring(), bool read_only=false, const tstring &file_type=tstring());

			LWODBC_API virtual tstring get_dbms_name() const { return tstring(); }
            LWODBC_API virtual tstring get_driver_name() const { return tstring(); }

			tstring LWODBC_API get_error_log(bool clear_error=true);

			enum {
				odbc_no_DQB=1,
				odbc_no_UID=2,
				odbc_no_PWD=4,
				odbc_no_SystemDB=8,
				odbc_no_DATABASE=0x10,
				odbc_open_read_only=0x20
			};

			virtual tstring get_odbc_connection_string(const tstring &uid, const tstring &pwd, int flags=0) const = 0;
			tstring get_odbc_connection_string(int flags=0) const
			{
				return get_odbc_connection_string(_T(""), _T(""), flags | odbc_no_UID | odbc_no_PWD);
			}

			///@name Creating users, groups, schemas and databases
			//@{
			/// create a new database user
			virtual sqlreturn create_user(connection *ds, const tstring &uid, const tstring &pwd) = 0;
			/// add a database user to a group
			virtual sqlreturn add_user(connection *ds, const tstring &user, const tstring &group) = 0;
			/// remove a user from a group or from the database if @p group is empty
			virtual sqlreturn drop_user(connection *ds, const tstring &uid, const tstring &group) = 0;

			/// create a new database group
			virtual sqlreturn create_group(connection *ds, const tstring &gid) = 0;
			/// drop a group from the database
			virtual sqlreturn drop_group(connection *ds, const tstring &gid) = 0;
			/// create a new database at the location specified by 'ds'
			virtual sqlreturn create_database(connection *ds, const tstring &database_name) = 0;
			/// create a new schema
			virtual sqlreturn create_schema(connection *ds, const tstring &schema_name) = 0;
			/// drop a schema
			virtual sqlreturn drop_schema(connection *ds, const tstring &schema_name) = 0;
			/// change the password
			virtual sqlreturn change_password(connection *ds, const tstring &oldpw, const tstring &newpw, const tstring &uid) = 0;

			enum standard_sql_statement {
				sql_change_password	///< return a statement with ([in]oldpw), ([in]newpw) and ([in]uid) bind markers
			};
			virtual tstring get_sql_for(standard_sql_statement /*s*/) const { return tstring(); }
			//@}

			virtual sqlreturn get_current_sequence_value(connection *ds, const accessor &target, const tstring &sequence_name, bool expand_sequence_name_from_column) = 0;

			/// use the following database from now on and also change the ODBC connection string to use it next time
			virtual sqlreturn use_database(connection *ds, const tstring &database_name) = 0;

			virtual void set_database_name(const tstring &database) = 0;
			virtual tstring get_database_name() const = 0;

			virtual tstring get_macro_value(const tstring &macro_name) const;
			virtual tstring get_macro_value(const tstring &macro_name, const tstring &macro_parameters) const;

			/// query the capabilities of the dbms
			virtual bool has_capability(capabilities c) const = 0;

			/// Give the dbms strategy the chance to override some capabilities as returned by ODBC
			/// This allows the strategy object to make special settings to avoid some of the ODBC driver idiosyncrasies,
			/// such as bugs in the MySQL ODBC driver for example.
			virtual void override_driver_capabilities(connection *) {}

			/// return the closest C data type for the SQL data type
			virtual SQLSMALLINT sql_to_c_type(SQLSMALLINT sql_type) const = 0;
			/// return a string that can be used for a column type in a create table statement
			virtual tstring sql_to_create_table_name(connection *ds, SQLSMALLINT sql_type, SQLLEN length) const = 0;

			const map<tstring, tstring> &macros() const { return m_macros; }

			using can_handle_func_t = bool (*)(const tstring &name, const tstring &version, const tstring &odbc_connection_string);
			using creator_func_t = dbms_base *(*)(const tstring &odbc_connection_string);
			using creator_t = vector<pair<can_handle_func_t, creator_func_t> >;
			static sqlreturn register_dbms(can_handle_func_t, creator_func_t, void *stop_linker_from_removing_me);

			struct do_register
			{
				do_register(can_handle_func_t h, creator_func_t f)
				{
					dbms_base::register_dbms(h, f, this);
				}
			};

			///! allow for idiosyncrasies of the specific odbc driver in the way it wants uuids bound to columns.
			///! the public postgresql odbc driver up to version 11.0 does not handle UUIDs correctly.
			virtual unsigned char get_uuid_variant(const unsigned char *uuid_data) const { return (uuid_data[8]&0xe0); }


		protected:
			static creator_t &get_dbms_register();
			map<tstring, tstring> &macros() { return m_macros; }
			map<tstring, boost::function<tstring(const tstring&)>> &macro_fncs() { return m_macro_fncs; }

			tstring get_sql_column_name(const const_accessor &a) const;
			tstring m_error_log;
			bool_result call_SQLConfigDataSource(SQLHWND hwnd,  const tstring &command);
			static tstring csvdelimiter();
			map<tstring, tstring> m_macros;
			map<tstring, boost::function<tstring(const tstring&)>> m_macro_fncs;
		};

		/** Generic DBMS strategy. Used if no other, more specific strategy is available. */
		class dbms_generic:public dbms_base
		{
		public:
			dbms_generic(const tstring &odbc_connection_string)
				:m_odbc_connection_string(odbc_connection_string)
			{
			}
			tstring get_odbc_connection_string(const tstring &uid, const tstring &pwd, int flags = 0) const override
			{
				tstring rc=m_odbc_connection_string;
				if ((flags & odbc_no_UID) == 0) {
					rc.append(_T(";UID=")).append(uid);
				}
				if ((flags & odbc_no_PWD) == 0) {
					rc.append(_T(";PWD=")).append(pwd);
				}
				if ((flags & odbc_no_DATABASE) == 0 && !m_database_name.empty()) {
					rc.append(_T(";DATABASE=")).append(m_database_name);
				}
				return rc;
			}
			sqlreturn create_user(connection *ds, const tstring &uid, const tstring &pwd) override;
			sqlreturn add_user(connection *ds, const tstring &user, const tstring &group) override;
			sqlreturn drop_user(connection *ds, const tstring &uid, const tstring &group) override;
			sqlreturn change_password(connection *ds, const tstring &oldpw, const tstring &newpw, const tstring &uid) override;

			sqlreturn create_group(connection *ds, const tstring &gid) override;
			sqlreturn drop_group(connection *ds, const tstring &gid) override;

			sqlreturn create_database(connection *ds, const tstring &database_name) override;
			sqlreturn create_schema(connection *ds, const tstring &schema_name) override;
			sqlreturn drop_schema(connection *ds, const tstring &schema_name) override;
			sqlreturn use_database(connection *ds, const tstring &database_name) override;
			sqlreturn get_current_sequence_value(connection *ds, const accessor &target, const tstring &sequence_name, bool expand_sequence_name_from_column) override;
			void set_database_name(const tstring &database) override
			{
				m_database_name=database;
			}
			tstring get_database_name() const override
			{
				return m_database_name;
			}
			bool has_capability(dbms_base::capabilities c) const override
			{
				static const int general_capabilities=(has_user_accounts | has_schema);
				return ((int)c & general_capabilities) == (int)c;
			}

			SQLSMALLINT sql_to_c_type(SQLSMALLINT sql_type) const override;
			tstring sql_to_create_table_name(connection *ds, SQLSMALLINT sql_type, SQLLEN length) const override;

			tstring get_sql_for(standard_sql_statement s) const override;
		protected:
			dbms_type_mapper m_type_mapper;
			tstring m_odbc_connection_string;
			tstring m_database_name;
		};

		/** MS-SQL Server 2000 strategy */
		class dbms_sql_server:public dbms_generic
		{
		public:
			dbms_sql_server(const tstring &odbcConnection=tstring());
			tstring get_dbms_name() const override { return _T("SQL Server"); }
			sqlreturn create_database(connection *ds, const tstring &database_name) override;
			sqlreturn create_user(connection *ds, const tstring &uid, const tstring &pwd) override;
			sqlreturn add_user(connection *ds, const tstring &user, const tstring &group) override;
			sqlreturn drop_user(connection *ds, const tstring &uid, const tstring &group) override;
			sqlreturn get_current_sequence_value(connection *ds, const accessor &target, const tstring &sequence_name, bool expand_sequence_name_from_column) override;
			sqlreturn create_group(connection *ds, const tstring &gid) override;
			sqlreturn drop_group(connection *ds, const tstring &gid) override;
			bool has_capability(capabilities c) const override { return dbms_generic::has_capability(c) || c==has_get_current_sequence_value; }

			static dbms_base *construct(const tstring &odbc_connection) { return new dbms_sql_server(odbc_connection); }
			static bool can_handle_(const tstring &name, const tstring &version, const tstring &odbc_connection_string);
		};

		/** MySQL 4.x strategy */
		class dbms_mysql:public dbms_generic
		{
		public:
			dbms_mysql(const tstring &odbcConnection=tstring());
			tstring get_dbms_name() const override { return _T("MySQL"); }
			static dbms_base *construct(const tstring &odbc_connection) { return new dbms_mysql(odbc_connection); }
			void override_driver_capabilities(connection *c) override;
			static bool can_handle_(const tstring &name, const tstring &version, const tstring &odbc_connection_string);
		};

		/** PostgreSQL 8.0 strategy */
		class dbms_postgres:public dbms_generic
		{
		public:
			dbms_postgres(const tstring &odbcConnection=tstring());
			tstring get_dbms_name() const override { return _T("PostgreSQL"); }
			static dbms_base *construct(const tstring &odbc_connection) { return new dbms_postgres(odbc_connection); }
			bool has_capability(capabilities c) const override;
			sqlreturn get_current_sequence_value(connection *ds, const accessor &target, const tstring &sequence_name, bool expand_sequence_name_from_column) override;
			static bool can_handle_(const tstring &name, const tstring &version, const tstring &odbc_connection_string);

			sqlreturn add_user(connection *ds, const tstring &user, const tstring &group) override;
			sqlreturn drop_user(connection *ds, const tstring &uid, const tstring &group) override;

			sqlreturn create_group(connection *ds, const tstring &gid) override;
			sqlreturn drop_group(connection *ds, const tstring &gid) override;

			///! The public postgres odbc drivers up to version 11 do not handle RFC_422 UUIDs correctly.
			///! They treat them as MS variant with the weird swapped byte order rather than storing them
			///! as "network" byte order.
			///! The binder queries this function to determine how it should bind the data.
			///! Returning 0xc0 in any case here forces the binder to treat all uuids as MS variant.
			unsigned char get_uuid_variant(const unsigned char * /*uuid_data*/) const override { return 0xc0; }
		};

		/** SQLite strategy */
		class dbms_sqlite :public dbms_generic
		{
		public:
			dbms_sqlite(const tstring &odbcConnection = tstring());
			tstring get_dbms_name() const override { return _T("SQLite"); }
			static dbms_base *construct(const tstring &odbc_connection) { return new dbms_sqlite(odbc_connection); }
			static bool can_handle_(const tstring &name, const tstring &version, const tstring &odbc_connection_string);

			virtual void override_driver_capabilities(connection *c) override;

		};

		/** Firebird 1.5 strategy */
		class dbms_firebird:public dbms_generic
		{
		public:
			dbms_firebird(const tstring &odbcConnection=tstring());
			tstring get_dbms_name() const override { return _T("Firebird"); }
			static dbms_base *construct(const tstring &odbc_connection) { return new dbms_firebird(odbc_connection); }
			static bool can_handle_(const tstring &name, const tstring &version, const tstring &odbc_connection_string);
			void override_driver_capabilities(connection *c) override;
		};

	};

};

#pragma warning (pop)
#endif
