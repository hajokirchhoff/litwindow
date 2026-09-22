#include "stdafx.h"
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>


#include "litwindow/dataadapter.h"
#include "litwindow/odbc/table.h"
#include "litwindow/odbc/connection.h"
#include "boost/lexical_cast.hpp"
#include "boost/optional/optional.hpp"
#include <sstream>

#define new DEBUG_NEW

using namespace litwindow;

// The "Driver=" value in an ODBC connection string must match the driver's
// registered name, i.e. the bracketed section name in odbcinst.ini (or the
// registry key on Windows) - NOT necessarily the human readable "Description=".
// On Windows, the SQLite ODBC installer (http://www.ch-werner.de/sqliteodbc/)
// registers the driver under the name "SQLite3 ODBC Driver".
// On Debian/Ubuntu, the "libsqlite3odbc" package registers it under the short
// section name "SQLite3" (run 'odbcinst -q -d' or check /etc/odbcinst.ini to confirm).
#if defined(_WIN32)
const tstring sqlite3_driver_name = _T("SQLite3 ODBC Driver");
#else
const tstring sqlite3_driver_name = _T("SQLite3");
#endif

const tstring sqLite3Connection = _T("Driver={") + sqlite3_driver_name + _T("};Database=v4.db;stepapi=0;notxn=0;shortnames=0;longnames=0;nocreat=0;nowchar=0;fksupport=1;oemcp=0;bigint=0;jdconv=0");

/// Print instructions on how to make the "SQLite3 ODBC Driver" available so that
/// the developer can fix their environment instead of just seeing an abrupt crash.
static std::string sqlite_odbc_driver_install_hint()
{
	std::ostringstream msg;
	msg << "Could not open a connection using the ODBC driver \"" << litwindow::t2string(sqlite3_driver_name) << "\".\n"
		<< "This test requires a SQLite ODBC driver to be installed and registered with the ODBC driver manager\n"
		<< "under exactly that name (the \"Driver=\" value in a connection string must match the driver's\n"
		<< "registered/section name, not necessarily its human-readable description).\n"
#if defined(_WIN32)
		<< "On Windows, install the driver from http://www.ch-werner.de/sqliteodbc/ "
		<< "(sqliteodbc.exe / sqliteodbc_w64.exe) and make sure \"SQLite3 ODBC Driver\" appears in the ODBC Data Source Administrator.\n";
#else
		<< "On Linux, install the unixODBC driver manager together with the SQLite ODBC driver, e.g.:\n"
		<< "  Debian/Ubuntu : sudo apt-get install unixodbc unixodbc-dev libsqliteodbc\n"
		<< "  Fedora/RHEL   : sudo dnf install unixODBC unixODBC-devel sqliteodbc\n"
		<< "  Arch Linux    : sudo pacman -S unixodbc libsqliteodbc\n"
		<< "The package registers the driver in /etc/odbcinst.ini (run 'odbcinst -j' to locate the\n"
		<< "config files and 'odbcinst -q -d' to list the registered driver names). Note that the\n"
		<< "bracketed section name (e.g. \"SQLite3\"), not the Description text, is what must be used\n"
		<< "as \"Driver=\" in the connection string. If your distribution registers a different section\n"
		<< "name, update 'sqlite3_driver_name' in odbc_unittest.cpp to match.\n";
#endif
	return msg.str();
}

/// Try to open the given connection, converting any exception into a graceful
/// Boost.Test failure with actionable installation instructions instead of an
/// uncaught exception that aborts the whole test process.
static bool try_open_sqlite_connection(odbc::connection &c, odbc::sqlreturn &rc)
{
	try {
		rc = c.open(sqLite3Connection);
	}
	catch (const std::exception &e) {
		BOOST_ERROR("Exception while opening SQLite ODBC connection: " << e.what() << "\n" << sqlite_odbc_driver_install_hint());
		return false;
	}
	catch (...) {
		BOOST_ERROR("Unknown exception while opening SQLite ODBC connection.\n" << sqlite_odbc_driver_install_hint());
		return false;
	}
	if (!rc.success()) {
		BOOST_ERROR("Failed to open SQLite ODBC connection: " << litwindow::t2string(rc.as_string()) << "\n" << sqlite_odbc_driver_install_hint());
		return false;
	}
	return true;
}

void odbc_create_empty_table_test()
{
	odbc::connection c;

	odbc::sqlreturn rc;
	BOOST_REQUIRE_MESSAGE(try_open_sqlite_connection(c, rc), "Aborting test: SQLite ODBC driver is not available. See message above for installation instructions.");

	{
		odbc::statement stmt(_T("SELECT * FROM test"), c);
		rc = stmt.execute();
		auto result = rc.as_string();

		SQLLEN count;
		rc = stmt.get_row_count(count);
	}

	odbc::statement stmt(c);
	stmt.set_statement(_T("DROP TABLE test"));
	rc = stmt.execute();

	stmt.set_statement(_T(R"(
CREATE TABLE test (
	id integer primary key,
	val real,
	textval text,
	optional_val real,
	optional_textval text
)
)"));
	rc = stmt.execute();

	BOOST_CHECK(rc.success());

	{
		odbc::statement stmt(_T("SELECT * FROM test"), c);
		rc = stmt.execute();
		BOOST_CHECK(rc.success());
		auto result = rc.as_string();

		SQLLEN count;
		rc = stmt.get_row_count(count);
		BOOST_CHECK(rc.success());
		// Die Tabelle wurde vorhin gel�scht und neu erzeugt. Sie muss 0 Zeilen enthalten.
		BOOST_CHECK_EQUAL(count, 0);
	}
}

/*! Hier eine einfache Struct.
 *!*/
struct test_struct
{
	int m_id;
	float m_val;
	tstring m_textval;
	boost::optional<float> m_optional_val;
	boost::optional<std::string> m_optional_textval;
};

/*! Hier das zugeh�rige Databinding.
 *! Ab jetzt kann man generisch auf die Elemente der Struct zugreifen mit "get_accessor/get_aggregate".
 *! Weitere Infos im litwindow dataadapter Unittest.
 *! */
LWL_BEGIN_AGGREGATE(test_struct)
PROP(m_optional_textval)
PROP(m_optional_val)
PROP(m_id)
PROP(m_val)
PROP(m_textval)
LWL_END_AGGREGATE()

void odbc_insert_rows()
{
	// Verwende ab jetzt den Connection Pool.
	odbc::connection::pool().set(sqLite3Connection);

	// Das spart uns, bei jedem Statement oder jeder Table eine Connection mitzuschleppen.
	odbc::table t(_T("test"));

	// Jetzt eine Variable f�r test_struct anlegen und mit Werten f�llen.
	test_struct my_struct{ 5, 9.5, _T("ein Test"), 413.f, string("optionaler string") };

	// Hier wird die Tabelle ge�ffnet und an die Variable gebunden.
	t.open(my_struct);

	// Da das Binding bereits passiert ist, kann man direkt insert_row aufrufen.
	// Select, Insert, Update und Delete Statements werden automatisch generiert.
	odbc::sqlreturn rc = t.insert_row();
	BOOST_CHECK(rc);
	auto as_string = rc.as_string();

	// Turn on exceptions
	odbc::connection::pool().get()->set_throw_on_error_default(true);
	// populate table
	for (int i = 0; i < 20; ++i) {
		my_struct.m_id = i;
		my_struct.m_val = float(i);
		my_struct.m_textval = boost::lexical_cast<tstring>(i);
		if ((i % 2) == 0)
			my_struct.m_optional_val = float(i) + 2;
		else
			my_struct.m_optional_val = boost::none;
		if ((i % 3) == 0)
			my_struct.m_optional_textval = boost::lexical_cast<std::string>(100 + i);
		else
			my_struct.m_optional_textval = boost::none;
		t.insert_row();
	}

}

void odbc_verify_rows()
{
	odbc::connection::pool().set(sqLite3Connection);
	// Turn on exceptions
	odbc::connection::pool().get()->set_throw_on_error_default(true);

	odbc::table t(_T("test"));

	test_struct my_struct;
	t.open(my_struct);

	int i = 0;
	while (t.fetch().success() && t.last_error().no_data() == false) {
		if (i == 5) {
			// Diese Zeile wurde oben zuallererst explizit eingef�gt.
			// Da m_val der Primary Key ist, wird sie beim neuerlichen insert_row
			// nicht �berschrieben.
			BOOST_CHECK_EQUAL(my_struct.m_val, 9.5f);
			BOOST_CHECK(my_struct.m_textval == _T("ein Test"));
			BOOST_CHECK(my_struct.m_optional_val == 413.f);
			BOOST_CHECK(my_struct.m_optional_textval == std::string("optionaler string"));
		}
		else {
			BOOST_CHECK_EQUAL(my_struct.m_id, i);
			BOOST_CHECK_EQUAL(my_struct.m_val, float(i));
			BOOST_CHECK(my_struct.m_textval == boost::lexical_cast<tstring>(i));
			if ((i % 2) == 0)
				BOOST_CHECK(my_struct.m_optional_val == boost::optional<float>(i + 2.0f));
			else
				BOOST_CHECK(my_struct.m_optional_val == boost::none);
			if ((i % 3) == 0)
				BOOST_CHECK(my_struct.m_optional_textval == boost::lexical_cast<std::string>(100 + i));
			else
				BOOST_CHECK(my_struct.m_optional_textval == boost::none);
		}
		++i;
	}
	BOOST_CHECK_EQUAL(i, 20);
}

BOOST_AUTO_TEST_CASE(simple_odbc_test)
{
	odbc_create_empty_table_test();
	odbc_insert_rows();
	odbc_verify_rows();
}
