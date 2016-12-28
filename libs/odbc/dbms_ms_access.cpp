/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: dbms_ms_access.cpp,v 1.5 2007/01/12 11:46:11 Hajo Kirchhoff Exp $
*/

#include "stdafx.h"
#include <iomanip>
#include <odbcinst.h>
#include <litwindow/dataadapter.h>
#include <litwindow/check.hpp>
#include <io.h>
#include "litwindow/odbc/dbms_ms_access.h"
#include "litwindow/odbc/statement.h"
#define new DEBUG_NEW

namespace litwindow {

	namespace odbc {

		namespace {
#include "./md5.cpp"
		};

		void dbms_access::init_macros()
		{
			macros()[_T("BOOLEAN")]=_T("BIT");
			macros()[_T("TRUE")]=_T("TRUE");
			macros()[_T("FALSE")]=_T("FALSE");
            macros()[_T("UUID")]=_T("GUID");
            macros()[_T("TIMESTAMP")]=_T("DATETIME");
		}

		void dbms_access::set_parameters_from_odbc_connection_string(const tstring &odbc_connection)
		{
			//TODO: Implement this: parse the connection string and set database file name and workgroup file name from it
		}
		sqlreturn dbms_access::get_current_sequence_value(connection *ds, const accessor &target, const tstring &sequence_name, bool expand_sequence_name_from_column) throw()
		{
			statement s(_("SELECT @@IDENTITY"), *ds);
			s.bind_column(1, target) && s.execute() && s.fetch();
			if (s.last_error().no_data()) {
				return sqlreturn(_("selecting @@IDENTITY returned no rows: ")+s.get_statement(), odbc::err_dbms_specific_error);
			}
			return s.last_error();
		}

		tstring dbms_access::get_odbc_connection_string(const tstring &uid, const tstring &pwd, int flags) const
		{
			tstring security;
			tstring readOnly;
			bool open_workgroup=use_workgroup();
			if (!open_workgroup && m_use_workgroup==with_workgroup_if_present) {
				// test for a workgroup file
				open_workgroup= _taccess(get_workgroup_db_name().c_str(), 0)==0 ? with_workgroup : without_workgroup;
			}
			if (open_workgroup && (flags & odbc_no_SystemDB)==0) {
				security=_T("SystemDB=") + get_workgroup_db_name();
			}

			if (!open_workgroup)    // don't use UID or PWD without a workgroup file
				flags|=(odbc_no_UID | odbc_no_PWD);

			tstring dbq;
			tstring useUid;
			tstring usePwd;
			if ((flags & odbc_no_DQB) == 0)
				dbq=_T("DBQ=")+get_data_db_name()+_T(";DefaultDir=")+get_data_db_directory()+_T(';');
			if ((flags & odbc_no_UID) == 0)
				useUid=_T("UID=")+ uid + _T(';');
			if ((flags & odbc_no_PWD) == 0)
				usePwd=_T("PWD=%s") + pwd + _T(';');
			if ((flags & odbc_open_read_only) != 0)
				readOnly=_T("READONLY=1;");
			return dbq + _T("Driver={Microsoft Access Driver (*.mdb)};DriverId=25;") + useUid + usePwd + readOnly
				+ _T("FIL=MS Access;MaxBufferSize=2048;MaxScanRows=8;PageTimeout=5;SafeTransactions=0;Threads=3;UserCommitSync=Yes;ExtendedAnsiSql=1;")
				+ security;
		}

		sqlreturn dbms_access::create_database(const tstring &admin_account, SQLHWND parent)
		{
			{ // remove an existing file, suppress error if not exists
				_tremove(get_data_db_name().c_str());
				_tremove(get_workgroup_db_name().c_str());
				if (_taccess(get_data_db_name().c_str(), 0)==0 || _taccess(get_workgroup_db_name().c_str(), 0)==0) {
					tstring err= _("The old database file '") + get_data_db_name() + _("' could not be removed. It could be locked by a different user.\n");
					m_error_log+=err;
					return sqlreturn(err, odbc::err_dbms_specific_error);
				}
			}
			// create MS-Access compatible database
			tstring connection_string;
			bool_result rc=true;
			if (use_workgroup()) {
				// create system database
				tstring cfg_parameter=_T("CREATE_SYSDB=\"") + get_workgroup_db_name() + _T("\";CREATE_DBV4=\"") + get_data_db_name() + _T("\" ENCRYPT;");
				rc=call_SQLConfigDataSource(parent, cfg_parameter);
				if (rc.failed())
					return  sqlreturn(_T("SQLConfigDataSource ")+cfg_parameter+_T(" failed."), odbc::err_dbms_specific_error);
				// now create the admin account so that we can use it to create the database and have the admin as owner
				{
					context_t c("creating admin user");
					connection db(new dbms_access(*this));
					Verify( db.open(get_odbc_connection_string(_T("admin"), tstring())).ok(), "opening database");
					// NOTE: It is tempting to set the privileges for the new users here but 
					// as we are going to delete the database again, we cannot do it.
					// So add the user and group only and do nothing else yet.
					Verify( db.create_group(_T("mk_admins_group")).ok()
						&& db.create_user(admin_account, tstring()).ok()
						&& db.add_user(admin_account, _T("mk_admins_group")).ok()
						&& db.add_user(admin_account, _T("admins")).ok()
						&& db.add_user(admin_account, _T("users")).ok_log()
						, "error creating admin (owner) account");
				}
				Verify(_tremove(get_data_db_name().c_str())==0, "could not remove temporary database file");
				connection_string=_T("SystemDB=") + get_workgroup_db_name() + _T(";UID=") + admin_account + _T(";CREATE_DBV4=\"") + get_data_db_name() + _T("\" ENCRYPT;");
				// create the new database
				rc=call_SQLConfigDataSource(parent, connection_string);
				if (rc.failed())
					return sqlreturn(_T("SQLConfigDataSource ")+connection_string+_T(" failed."), odbc::err_dbms_specific_error);
				// and finally grant all rights to the new admin group, but remove all rights from the default 'admin', 'admins', 'users'
				// here is the right place - after the database has been created for good - to set the access rights
				{
					context_t c("setting basic access rights");
					odbc::connection db(new dbms_access(*this));
					Verify( db.open(get_odbc_connection_string(admin_account, tstring())).ok(), "opening database");
					// now grant all privileges to our own administrator and remove all privileges from the default one
					Verify( db.execute(

						_T("GRANT ALL PRIVILEGES ON DATABASE TO mk_admins_group;")
						_T("GRANT ALL PRIVILEGES ON DATABASE TO ") + admin_account + _T(";")
						_T("REVOKE ALL PRIVILEGES ON DATABASE FROM PUBLIC;")
						_T("REVOKE ALL PRIVILEGES ON DATABASE FROM admins;")
						_T("REVOKE ALL PRIVILEGES ON DATABASE FROM admin;COMMIT")
						).ok_log(), "setting privileges");

				}
			} else {
				connection_string=_T("CREATE_DBV4=\"") + get_data_db_name() + _T("\"\0\0");
				rc=call_SQLConfigDataSource(parent, connection_string);
				if (rc.failed())
					return sqlreturn(_T("SQLConfigDataSource ")+connection_string+_T(" failed."), odbc::err_dbms_specific_error);
			}
			return sqlreturn(SQL_SUCCESS);
		}

		sqlreturn dbms_access::create_database(connection *ds, const tstring &database_name)
		{
			// must use create_database(const tstring &filename, wxWindow *parent) instead!!!!
			Fail("creating a database by name is not supported for the MS-Access DBMS system");
			return sqlreturn(SQL_ERROR);
		}
		sqlreturn dbms_access::use_database(connection *, const tstring &)
		{
			// must use create_database(const tstring &filename, wxWindow *parent) instead!!!!
			//        Fail("using a database by name is not supported for the MS-Access DBMS system");
			//return false;
			return sqlreturn(SQL_SUCCESS);
		}
		void dbms_access::set_database_name(const tstring&)
		{
			Fail("MS-Access does not support using databases by name. Use set_data_db_name instead to set the filename.");
		}
		tstring dbms_access::get_database_name() const
		{
			Fail("MS-Access does not support using databases by name. Use get_data_db_name instead to get the filename.");
			return tstring();
		}

		sqlreturn dbms_access::create_user_or_group(connection *ds, const tstring &uid, const tstring &pwd, bool create_group)
		{
			tstring pid(_T("01234567890123456789"));
			unsigned char md5[16];
			get_md5(uid.c_str(), (int)(uid.length()*sizeof(uid[0u])), md5);
			size_t i;
			static const TCHAR set[]=_T("0123456789abcdef");
			for (i=0; i<pid.size(); ++i) {
				pid[i]=set[ (md5[i/2] >> ((i&1)==1 ? 4 : 0) ) & 0xf ];
			}
			sqlreturn rc;
			if (create_group)
				rc=ds->execute(_T("CREATE GROUP '") + uid + _T("' ") + pid);
			else
				rc=ds->execute(_T("CREATE USER '") + uid + _T("' '") + pwd + _T("' ") + pid);
			return rc;
		}

		tstring dbms_access::get_data_db_directory() const
		{
			size_t p=m_data_db_file.find_last_of(_T('\\'));
			if (p==tstring::npos)
				p=m_data_db_file.find_last_of(_T('/'));
			return p==tstring::npos ? _T(".") : m_data_db_file.substr(0, p);
		}

		void dbms_access::set_workgroup_db_name(const tstring &db_name)
		{
			m_workgroup_db_file=db_name;
			size_t p=m_workgroup_db_file.find_last_of(_T('.'));
			if (p!=tstring::npos)
				m_workgroup_db_file.erase(p);
			m_workgroup_db_file.append(_T(".mdw"));
		}

		//-----------------------------------------------------------------------------------------------------------//
		//-----------------------------------------------------------------------------------------------------------//
		//#region Access create user statements
		sqlreturn dbms_access::create_user(connection *ds, const tstring &uid, const tstring &pwd)
		{
			return use_workgroup() ? create_user_or_group(ds, uid, pwd, false) : sqlreturn(SQL_SUCCESS);
		}

		sqlreturn dbms_access::create_group(connection *ds, const tstring &gid)
		{
			return use_workgroup() ? create_user_or_group(ds, gid, tstring(), true) : sqlreturn(SQL_SUCCESS);
		}

		sqlreturn dbms_access::add_user(connection *ds, const tstring &user, const tstring &group)
		{
			return use_workgroup() ? ds->execute(_T("ADD USER '") + user + _T("' TO '") +  group + _T("'")) : sqlreturn(SQL_SUCCESS);
		}

		sqlreturn dbms_access::drop_user(connection *ds, const tstring &user, const tstring &group)
		{
			return use_workgroup() ? ds->execute(_T("DROP USER '") + user + _T("'") + ( group.size()>0 ? _T(" FROM '")+group+_T("'") : _T("") ) ) : sqlreturn(SQL_SUCCESS);
		}

		sqlreturn dbms_access::drop_group(connection *ds, const tstring &gid)
		{
			return use_workgroup() ? ds->execute(_T("DROP GROUP '") + gid + _T("'")) : sqlreturn(SQL_SUCCESS);
		}
		//#endregion

		bool dbms_access::can_handle_(const tstring &name, const tstring &version, const tstring &odbc_connection_string)
		{
			return name==_T("Microsoft Access Driver (*.mdb)") || name==_T("ACCESS");
		}


		namespace {
			dbms_base::do_register ms_access_driver(dbms_access::can_handle_, dbms_access::construct);
		};

	};

};