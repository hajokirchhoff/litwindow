#include "stdafx.h"
#define BOOST_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>


#include "litwindow/dataadapter.h"
#include "litwindow/odbc/table.h"
#include "litwindow/odbc/connection.h"

#define new DEBUG_NEW

using namespace litwindow;

const wstring sqLite3Connection = L"Driver={SQLite3 ODBC Driver};DATABASE=X:\\V4t.dat;stepapi=0;notxn=0;shortnames=0;longnames=0;nocreat=0;nowchar=0;fksupport=0;oemcp=0;bigint=0;jdconv=0";

void odbc_test_1()
{
	odbc::connection c;

	odbc::sqlreturn rc=c.open(sqLite3Connection);
	BOOST_CHECK(rc.success());

	odbc::statement stmt;
	stmt.set_statement(L"DROP TABLE test");
	rc = stmt.execute();

	stmt.set_statement(LR"(
CREATE TABLE test (
	id int primary key,
	val real,
	textval text
)
)");
	rc = stmt.execute();

	BOOST_CHECK(rc.success());
}

BOOST_AUTO_TEST_CASE(simple_odbc_test)
{
	odbc_test_1();
}
