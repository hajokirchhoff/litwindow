#include "stdafx.h"
#define BOOST_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>


#include "litwindow/dataadapter.h"
#include "litwindow/odbc/table.h"
#include "litwindow/odbc/connection.h"
#include "boost/lexical_cast.hpp"

#define new DEBUG_NEW

using namespace litwindow;

const wstring sqLite3Connection = L"Driver={SQLite3 ODBC Driver};Database=v4.db;stepapi=0;notxn=0;shortnames=0;longnames=0;nocreat=0;nowchar=0;fksupport=0;oemcp=0;bigint=0;jdconv=0";

void odbc_create_empty_table_test()
{
	odbc::connection c;

	odbc::sqlreturn rc=c.open(sqLite3Connection);
	BOOST_CHECK(rc.success());

	{
		odbc::statement stmt(L"SELECT * FROM test", c);
		rc = stmt.execute();
		auto result = rc.as_string();

		SQLLEN count;
		rc = stmt.get_row_count(count);
	}

	odbc::statement stmt(c);
	stmt.set_statement(L"DROP TABLE test");
	rc = stmt.execute();

	stmt.set_statement(LR"(
CREATE TABLE test (
	id integer primary key,
	val real,
	textval text
)
)");
	rc = stmt.execute();

	BOOST_CHECK(rc.success());

	{
		odbc::statement stmt(L"SELECT * FROM test", c);
		rc = stmt.execute();
		BOOST_CHECK(rc.success());
		auto result = rc.as_string();

		SQLLEN count;
		rc = stmt.get_row_count(count);
		BOOST_CHECK(rc.success());
		// Die Tabelle wurde vorhin gelöscht und neu erzeugt. Sie muss 0 Zeilen enthalten.
		BOOST_CHECK_EQUAL(count, 0);
	}
}

/*! Hier eine einfache Struct.
 *!*/
struct test_struct
{
	int m_id;
	float m_val;
	wstring m_textval;
};

/*! Hier das zugehörige Databinding.
 *! Ab jetzt kann man generisch auf die Elemente der Struct zugreifen mit "get_accessor/get_aggregate".
 *! Weitere Infos im litwindow dataadapter Unittest.
 *! */
LWL_BEGIN_AGGREGATE(test_struct)
PROP(m_id)
PROP(m_val)
PROP(m_textval)
LWL_END_AGGREGATE()

void odbc_insert_rows()
{
	// Verwende ab jetzt den Connection Pool.
	odbc::connection::pool().set(sqLite3Connection);

	// Das spart uns, bei jedem Statement oder jeder Table eine Connection mitzuschleppen.
	odbc::table t(L"test");

	// Jetzt eine Variable für test_struct anlegen und mit Werten füllen.
	test_struct my_struct{ 5, 9.5f, L"ein Test" };

	// Hier wird die Tabelle geöffnet und an die Variable gebunden.
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
		my_struct.m_textval = boost::lexical_cast<std::wstring>(i);
		t.insert_row();
	}

}

BOOST_AUTO_TEST_CASE(simple_odbc_test)
{
	odbc_create_empty_table_test();
	odbc_insert_rows();
}
