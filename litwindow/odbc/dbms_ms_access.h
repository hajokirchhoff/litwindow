/** \file
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: dbms_ms_access.h,v 1.4 2006/07/25 07:27:15 Hajo Kirchhoff Exp $
*/

#ifndef __LWODBC_DBMS_MS_ACCESS_H
#define __LWODBC_DBMS_MS_ACCESS_H

#pragma once

#include "./dbms.h"

#pragma warning(push, 4)

// now make intellisense work again by undefining LWODBC_API for intellisense
#ifdef INTELLISENSE_DOES_NOT_WORK_PSEUDO_DEFINE
#undef LWODBC_API
#define LWODBC_API
#endif

namespace litwindow {

	namespace odbc {

		using namespace std;

#if defined(_WIN32) || defined(DOXYGEN_INVOKED)

#define DBMS_ACCESS_DEFINED

		/** This class exists only under Windows */
		/** MS-Access or MS-Jet ODBC class */
		class dbms_access:public dbms_generic
		{
		public:
			enum use_workgroup_enum {
				without_workgroup,
				with_workgroup,
				with_workgroup_if_present
			};
			LWODBC_API dbms_access(const tstring &db_file_name, use_workgroup_enum workgroup)
				:dbms_generic(tstring())
				,m_use_workgroup(workgroup)
				,m_data_db_file(db_file_name)
			{
				set_workgroup_db_name(db_file_name);
				init_macros();
			}
			/// create a new database user
			virtual sqlreturn create_user(connection *ds, const tstring &uid, const tstring &pwd);
			/// add a database user to a group
			virtual sqlreturn add_user(connection *ds, const tstring &user, const tstring &group);
			/// remove a user from a group
			virtual sqlreturn drop_user(connection *ds, const tstring &uid, const tstring &group);

			/// create a new database group
			virtual sqlreturn create_group(connection *ds, const tstring &gid);
			/// drop a group from the database
			virtual sqlreturn drop_group(connection *ds, const tstring &gid);

			/// create a new database and make 'admin_account' the owner
			sqlreturn LWODBC_API create_database(const tstring &admin_account, SQLHWND);

			/// Creating a database by name is not supported. This function will throw an error.
			virtual sqlreturn create_database(connection *ds, const tstring &database_name);
			virtual sqlreturn use_database(connection *ds, const tstring &database_name);
			virtual void set_database_name(const tstring &database);
			virtual tstring get_database_name() const;
			virtual sqlreturn get_current_sequence_value(connection *ds, const accessor &target, const tstring &sequence_name, bool expand_sequence_name_from_column);

			virtual tstring get_driver_name() const { return _T("Microsoft Access Driver (*.mdb)"); }

			/// get the filename of the database
			tstring get_data_db_name() const        { return m_data_db_file; }
			/// get the directory of the database
			tstring get_data_db_directory() const;
			/// get the filename of the workgroup file
			tstring get_workgroup_db_name() const   { return m_workgroup_db_file; }
			void set_data_db_name(const tstring &db_name)
			{
				m_data_db_file=db_name;
				set_workgroup_db_name(db_name);
			}
			tstring get_odbc_connection_string(const tstring &uid, const tstring &pwd, int flags=0) const;
			using dbms_base::get_odbc_connection_string;

			bool has_capability(capabilities c) const
			{
				if (c==has_user_accounts) return use_workgroup();
				if (c==has_get_current_sequence_value)
					return true;
				return false;
			}

			static dbms_base *construct(const tstring &odbc_connection) { return new dbms_access(odbc_connection); }
			static bool can_handle_(const tstring &name, const tstring &version, const tstring &odbc_connection_string);

			tstring get_dbms_name() const { return _T("MS-Jet"); }
		protected:
			LWODBC_API dbms_access(const tstring &odbc_connection_string)
				:dbms_generic(odbc_connection_string)
			{
				set_parameters_from_odbc_connection_string(odbc_connection_string);
				init_macros();
			}
			void init_macros();
			sqlreturn create_user_or_group(connection *ds, const tstring &uid, const tstring &pwd, bool create_group);

			/// determine if a workgroup file is being used
			use_workgroup_enum m_use_workgroup;
			bool use_workgroup() const { return m_use_workgroup==with_workgroup; }

			void set_workgroup_db_name(const tstring &db_name);
			///@todo use the boost::filesystem::path class once it supports unicode
			tstring m_data_db_file;
			tstring m_workgroup_db_file;

			void set_parameters_from_odbc_connection_string(const tstring &odbc_connection);

		};
#endif

	};

};

#pragma warning (pop)
#endif
