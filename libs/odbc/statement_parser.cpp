/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: statement_parser.cpp,v 1.4 2006/09/24 11:26:56 Hajo Kirchhoff Exp $
*/

#include "stdafx.h"

#ifdef _DEBUG
//#define BOOST_SPIRIT_DEBUG
#endif


#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/classic_actor.hpp>
#include <boost/bind.hpp>
#include <malloc.h>
#include <litwindow/logging.h>
#include "litwindow/odbc/statement.h"


using namespace boost::spirit::classic;

#pragma warning(disable: 4312 4267)

namespace litwindow {

namespace odbc {

#define new DEBUG_NEW

typedef scanner<const TCHAR*> scanner_t;
typedef rule<scanner_t > trule;


//distinct_parser<wide_phrase_scanner_t> keyword_p(_T("a-zA-Z0-9_"));

void add_char(tstring &t, TCHAR c)
{
	t.append(1, c);
}

void assign_tstring(tstring *array[4], int index, const tstring &value)
{
	*array[index]=value;
}
pair<scope, tstring> parse_scope(const tstring &full_name)
{
	pair<scope, tstring> rc;
	tstring last_identifier;
	tstring *full_scope[4]={
		&rc.first.catalog,
		&rc.first.schema,
		&rc.first.table,
		&rc.second
	};
	int current=0;

	trule plain_identifier= ((alpha_p | _T('_')) >> *(alnum_p | _T('_')))[assign_a(last_identifier)];

	trule quoted_identifier_char =  (
			str_p(_T("\"\""))[boost::bind(&add_char, boost::ref(last_identifier), _T('"'))]
			| (~ch_p(_T('"')))[boost::bind(&add_char, boost::ref(last_identifier), _1)]
			);

	trule quoted_identifier= eps_p[assign_a(last_identifier, _T(""))]
		>> confix_p(ch_p(_T('"')), *quoted_identifier_char, ch_p(_T('"')));

	trule identifier= quoted_identifier | plain_identifier;
	trule assign_identifier = identifier[boost::bind(&assign_tstring, full_scope, boost::ref(current), boost::ref(last_identifier))][increment_a(current)];
	trule scope= assign_identifier >> repeat_p(0, 3)[*space_p >> ch_p(_T('.')) >> *space_p >> assign_identifier];
	parse_info<const TCHAR* > info=parse(full_name.c_str(), scope);
	if (info.full==false) {
		throw runtime_error("invalid column identifier");
	}
	if (current<4) {
		int i;
		for (i=3; i>=0; --i) {
			*(full_scope[i])= current>0 ? *(full_scope[--current]) : tstring();
		}
	}
	return rc;
}

struct add_macro {
	add_macro(tostringstream &o, statement &s):m_o(o), m_s(s){}
	void operator() (const TCHAR *a, const TCHAR *b) const
	{
		m_o << m_s.get_macro_value(tstring(a,b));
	}
	tostringstream &m_o;
	statement &m_s;
};

struct add_output {
	add_output(tostringstream &o):m_o(o){}
	void operator() (const TCHAR *a, const TCHAR *b) const
	{
		m_o << tstring(a, b);
	}
	tostringstream &m_o;
	void operator() (const TCHAR a) const
	{
		m_o << a;
	}
};


class my_symbols:public symbols<size_t, TCHAR>
{
};

void statement::add_bind_marker(my_symbols &sym, parameter &p, size_t &col_no, size_t &param_no, const TCHAR *start, const TCHAR *end)
{
	vector<parameter> &list(p.m_bind_type==bindto ? m_column_marker : m_parameter_marker);
	size_t index=p.m_bind_type==bindto ? col_no : param_no;
	if (list.size()<=index)
		list.resize(index+1);
	else if (list[index].m_bind_type!=unknown_bind_type)
		throw_(start, _("marker for parameter or column already exists"));
	if (add(sym, p.m_parameter_name.c_str(), index)==NULL) {
		throw_(start, _("parameter or column marker used twice"));
	}
	p.m_position=index;
	list[index]=p;            
}

bool statement::do_parse_bindings()
{
	bool rc;
	try {
		m_column_marker.clear();
		m_parameter_marker.clear();

		size_t macro_count;
		do {
			TCHAR openQuote;
			parameter new_parameter;
			tostringstream out_stream;
			size_t next_column_number=1;
			size_t next_parameter_number=1;
			my_symbols sym;

			trule quote_char = ch_p(_T('"')) | _T('\'') | _T('`') | _T('´');
			trule escape_quote = ch_p(boost::ref(openQuote) >> boost::ref(openQuote));
			trule string_char = * (escape_quote | ~ch_p(boost::ref(openQuote))) ;
			trule r_string_double_quotes = ( quote_char[assign_a(openQuote)], string_char, ch_p(boost::ref(openQuote)) );

			trule simple_id = +(alnum_p | ch_p(_('_')));
			trule macro_id = (str_p(_("$$")) >> simple_id)[increment_a(macro_count)];

			trule numbers = +digit_p;
			trule r_operator = ~alnum_p;

			assertion<const TCHAR*> expect_bind_type(_T("bind type must be one of [bindto, in, out, inout]"));
			trule bound_type = expect_bind_type(    
				as_lower_d[_("bindto")] [boost::bind(&parameter::set_bind_type, boost::ref(new_parameter), odbc::bindto)]
			|   as_lower_d[_("in")]     [boost::bind(&parameter::set_bind_type, boost::ref(new_parameter), odbc::in)]
			|   as_lower_d[_("out")]    [boost::bind(&parameter::set_bind_type, boost::ref(new_parameter), odbc::out)]
			|   as_lower_d[_("inout")]  [boost::bind(&parameter::set_bind_type, boost::ref(new_parameter), odbc::inout)] );

			trule bound_to = (ch_p(_('(')) >> *space_p >> _('[') >> *space_p >> bound_type >> *space_p >> _(']') >> *space_p >> simple_id[assign_a(new_parameter.m_parameter_name)] >> *space_p>> _(')'))
				[boost::bind(&statement::add_bind_marker, this, boost::ref(sym), boost::ref(new_parameter), boost::ref(next_column_number), boost::ref(next_parameter_number), _1, _2)];

			trule column_separator = ch_p(_(','));

			trule stmt = *(
				(bound_to)
				|               ((ch_p(_('?')) >> *space_p)[add_output(out_stream)] >> !bound_to)[increment_a(next_parameter_number)]
			|               column_separator[add_output(out_stream)][increment_a(next_column_number)]
			|			macro_id[add_macro(out_stream, *this)]
			|               anychar_p[add_output(out_stream)]
			);

			//*( r_string_double_quotes | (simple_id >> !bound_to) | (ch_p(_('?')) >> !bound_to)  | numbers | column_separator | r_operator);
			BOOST_SPIRIT_DEBUG_NODE(bound_type);
			BOOST_SPIRIT_DEBUG_NODE(bound_to);
			BOOST_SPIRIT_DEBUG_NODE(stmt);

			rule<scanner<const TCHAR*> > my_skipper = space_p;

			macro_count=0;
			rc=parse(m_original_sql_statement.c_str(), stmt).full;
			m_parsed_sql_statement=out_stream.str();
		} while (rc && macro_count>0);
	}
	catch (parser_error<const TCHAR*, const TCHAR*> &e) {
		rc=false;
		lw_log() << e.descriptor << _(": ") << tstring(e.where, 20) << _("...") << endl;
		m_last_error=sqlreturn(e.descriptor, odbc::err_cannot_bind_column_twice);
	}
	return rc;

	////trule r_string_double_quotes = confix_p(ch_p(_T('"')), str_p(_T("hi")), ch_p(_T('"')));



	//tstring b_type;
	//tstring n_name;

	//trule simple_id = (+alnum_p)[assign_a(n_name)];
	//trule quoted_id = confix_p(ch_p(_('"')), +alnum_p, ch_p(_('"')));
	//trule id = simple_id | quoted_id;
	//trule compound_id = id % _('.');
	//trule name = ( compound_id >> ! (ch_p(_('.')) >> _('*')) ) | _('*');
	//trule prebound_name = name | ch_p(_('?'));
	//trule bound_name = prebound_name >> ! bound_to;

	//trule unknown_stuff = +graph_p;

	//trule column_list = bound_name % ( _(',') >> ! unknown_stuff);
	//trule stmt = (+unknown_stuff) >> column_list;

	////bool r=parse(_(" test \"test\".test"), simple_id, space_p).hit;

	////parse(_("* from test"), ch_p(_('*')));

	//return parse(m_sql_statement.c_str(), stmt, space_p).hit;
	////return false;
}


};

};
