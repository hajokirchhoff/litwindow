/* 
* Copyright 2005-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window ODBC Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window ODBC Library 
* distribution, file LICENCE.TXT
* $Id: connection_script_parser.cpp,v 1.7 2006/12/30 13:34:09 Hajo Kirchhoff Exp $
*/

#include "stdafx.h"
#include <sqlext.h>
#include <litwindow/logging.h>

//#include <boost/spirit.hpp>
//#include <boost/spirit/actor.hpp>

#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/home/classic/actor.hpp>
#include <boost/spirit/include/classic_iterator.hpp>
#include <boost/spirit/include/classic_core.hpp>

#include <boost/bind.hpp>
#include "litwindow/odbc/connection.h"
#include "litwindow/odbc/statement.h"

#define new DEBUG_NEW

namespace litwindow {

namespace odbc {

using namespace boost::spirit;

typedef boost::spirit::classic::position_iterator<const TCHAR*> sql_grammar_iterator_t;

namespace {

	typedef boost::spirit::classic::parser_error<tstring, sql_grammar_iterator_t> action_failed;

	struct parser_actions:public boost::noncopyable {
		parser_actions(connection &operate_on)
			:m_connection(operate_on)
			,m_repeat_state(normal_state)
		{

		}
		connection &m_connection;
		bool    m_ignore_error;
		tstring m_current_statement;
		tstring m_repeat_until_statement;
		enum repeat_state_enum {
			repeat_state,
			begin_state,
			normal_state
		} m_repeat_state;
		size_t  m_value_marker_position;

		void statement(sql_grammar_iterator_t first, sql_grammar_iterator_t last);
		void macro(sql_grammar_iterator_t first, sql_grammar_iterator_t last)
		{
			tstring value=m_connection.get_dbms()->get_macro_value(tstring(first, last));
			m_current_statement.append(value);
		}
		void out(sql_grammar_iterator_t first, sql_grammar_iterator_t last)
		{
			m_current_statement.append(first, last);
		}
		void ignore_error(bool yes)
		{
			m_ignore_error=yes;
		}
		void repeat(repeat_state_enum r)
		{
			m_repeat_state=r;
			if (r==repeat_state) {
				m_repeat_until_statement.clear();
				m_value_marker_position=tstring::npos;
			} else if (r==begin_state)
				m_repeat_until_statement=m_current_statement;
		}
		void repeat_value_marker()
		{
			m_value_marker_position=m_current_statement.length();
		}
		void begin_statement()
		{
			m_ignore_error=false;
			m_current_statement.reserve(256);
			m_current_statement.clear();
		}
		void log_unknown_input(sql_grammar_iterator_t first, sql_grammar_iterator_t last)
		{
			lw_log() << _T("Unknown token: ") << tstring(first, last) << endl;
		}
		void log_(sql_grammar_iterator_t first, sql_grammar_iterator_t last, const TCHAR *log_prefix)
		{
			lw_log() << log_prefix << tstring(first, last) << endl;
		}
	};
};

template <typename Actions>
struct script_grammar:public boost::spirit::classic::grammar<script_grammar<Actions> >,boost::noncopyable
{
	template <typename ScannerT>
	struct definition
	{
		definition(script_grammar const &self)
		{
			using namespace boost::spirit::classic;

			Actions &actions=self.actions;

			skip_p
				=   *boost::spirit::classic::space_p;

			unquoted_identifier
				=   ( boost::spirit::classic::alpha_p | _T('#') | _T('_') )
				>> * (boost::spirit::classic::alnum_p | _T('#') | _T('$') | _T('_'));

			quoted_identifier 
				=   boost::spirit::classic::confix_p(ch_p(_T('"')), *(~ch_p(_T('"'))) % str_p(_T("\"\"")), _T('"'));

			identifier
				=   (
                (quoted_identifier | unquoted_identifier) 
			% (skip_p >> _T('.') >> skip_p))[boost::bind(&Actions::log_, &actions, _1, _2, _T("identifier: "))]
				;

			uinteger
				=   +boost::spirit::classic::digit_p;
			ureal
				=   +boost::spirit::classic::digit_p >> skip_p >> boost::spirit::classic::ch_p(_T('.')) >> skip_p >> *boost::spirit::classic::digit_p;

			number
				=   boost::spirit::classic::longest_d[ ureal | uinteger ];

			string_character
				=   boost::spirit::classic::str_p(_T("''"))     // two single quotes escape a quote
				|   ~boost::spirit::classic::ch_p(_T('\''));    // any non-quote character

			string_literal 
				=   boost::spirit::classic::confix_p(ch_p(_T('\'')), *string_character, _T('\''));

			comment
				=   ( str_p(_T("--")) >> *anychar_p >> eol_p)
				|   confix_p(str_p("/*"), *anychar_p, str_p("*/"));

			macro_name
				=   identifier[boost::bind(&Actions::macro, &actions, _1, _2)];

			macro 
				=   ( boost::spirit::classic::str_p(_T("$$")) >> macro_name )
				|   boost::spirit::classic::confix_p(boost::spirit::classic::str_p(_T("$(")), macro_name, boost::spirit::classic::ch_p(_T(')')));

			statement_separator
				=   boost::spirit::classic::ch_p(_T(';')) 
				|   (_T('!') >> *boost::spirit::classic::blank_p >> boost::spirit::classic::eol_p);

			odbc_escape_type
				=   unquoted_identifier;

			operator_token
				=   boost::spirit::classic::ch_p(_T('+'))
				|   _T('-')
				|   _T('/')
				|   _T('*')
				|   _T('<')
				|   _T('>')
				|   _T('=')
				|   _T('?')
				|   _T('(')
				|   _T(')')
				|   _T(',')
				|   (boost::spirit::classic::ch_p(_T('<')) >> skip_p >> _T('='))
				|   (boost::spirit::classic::ch_p(_T('>')) >> skip_p >> _T('='))
				|   (boost::spirit::classic::ch_p(_T('<')) >> skip_p >> _T('>'))
				;

			odbc_escape_body
				=   * ((    string_literal
				|   identifier
				|   number
				|   operator_token
				|   ( (~boost::spirit::classic::ch_p(_T('}')))>> boost::spirit::classic::eps_p[boost::bind(&Actions::log_unknown_input, &actions, _1, _2)] )
				) 
				>> skip_p);

			odbc_escape
				=   boost::spirit::classic::confix_p(ch_p(_T('{')) >> skip_p, odbc_escape_type >> skip_p >> odbc_escape_body >> skip_p, boost::spirit::classic::ch_p(_T('}')));

			repeat_until_sequence
				=       str_p(_T("$#REPEAT"))[boost::bind(&Actions::repeat, &actions, parser_actions::repeat_state)] >> skip_p
				>>      statement >> skip_p >> statement_separator >> skip_p
				>>      confix_p(str_p(_T("$#BEGIN"))[boost::bind(&Actions::repeat, &actions, parser_actions::begin_state)] >> skip_p,
				+(statement >> skip_p >> statement_separator >> skip_p),
				str_p(_T("$#END"))[boost::bind(&Actions::repeat, &actions, parser_actions::normal_state)]);

			repeat_value_marker
				=       str_p(_T("$#VALUES"));

			unknown_token
				=       ( graph_p - (statement_separator | str_p(_T("$#"))));

			token
				=     ( macro
				| repeat_value_marker               [boost::bind(&Actions::repeat_value_marker, &actions)]
				| identifier                        [boost::bind(&Actions::out, &actions, _1, _2)]
				| string_literal                    [boost::bind(&Actions::out, &actions, _1, _2)]
				| number                            [boost::bind(&Actions::out, &actions, _1, _2)]
				| odbc_escape                       [boost::bind(&Actions::out, &actions, _1, _2)]
				| operator_token                    [boost::bind(&Actions::out, &actions, _1, _2)]
				| unknown_token                     [boost::bind(&Actions::out, &actions, _1, _2)][boost::bind(&Actions::log_unknown_input, &actions, _1, _2)]
				) >> skip_p                           [boost::bind(&Actions::out, &actions, _1, _2)];

				tokens
					=   +token;

				statement 
					=   eps_p[boost::bind(&Actions::begin_statement, &actions)] >> skip_p
					>>  (! ch_p('-')[boost::bind(&Actions::ignore_error, &actions, true)] >> skip_p )
					>>  tokens[boost::bind(&Actions::statement, &actions, _1, _2)];

				script 
					=   !( statement >> skip_p )
					>>  *(  ( repeat_until_sequence | statement_separator ) >> skip_p
					>> ! ( statement >> skip_p ) );
		}
		boost::spirit::classic::rule<ScannerT> const &start() const { return script; }
		boost::spirit::classic::rule<ScannerT>  script, tokens, statement, string_literal, identifier, unquoted_identifier, quoted_identifier, macro,
			statement_separator, comment, token, skip_p, macro_name, string_character, odbc_escape, odbc_escape_type, odbc_escape_body, number,
			operator_token, uinteger, ureal, repeat_until_sequence, repeat_value_marker, unknown_token;
	};

	script_grammar(Actions *actions_)
		:actions(*actions_)
	{

	}
	Actions &actions;
};

void parser_actions::statement(sql_grammar_iterator_t first, sql_grammar_iterator_t last)
{
	if (m_connection.unit_test_mode_()==1) {
		m_connection.unit_test_result_().append(m_current_statement).append(1, _T('\n'));
	} else {
		if (first==last) {
			lw_log() << _T("empty statement") << endl;
		} else if (m_repeat_state!=repeat_state) {
			if (m_repeat_state==begin_state && m_value_marker_position!=tstring::npos) {
				m_current_statement=m_repeat_until_statement.substr(0, m_value_marker_position)+m_current_statement+m_repeat_until_statement.substr(m_value_marker_position);
			}
			odbc::statement s(m_current_statement, m_connection);
			if (m_ignore_error)
				s.set_throw_on_error(false);
			if (s.execute().success()==false && m_ignore_error==false) {
				m_connection.set_last_error(s.last_error());
                throw action_failed(first, _T("SQL execute failed: ")+s.last_error().as_string());
			}
		}
	}
}

sqlreturn connection::execute(const litwindow::tstring &script)
{
	m_last_error.clear();
	parser_actions actions(*this);
	script_grammar<parser_actions> g(&actions);
	boost::spirit::classic::parse_info<sql_grammar_iterator_t> pi;
	sql_grammar_iterator_t begin(script.c_str(), script.c_str()+script.length());
	sql_grammar_iterator_t end;
	int last_line, last_column;
	last_line=last_column=0;
	try {
		pi= boost::spirit::classic::parse(begin, end, g);
		if (pi.full==false)
			set_last_error(sqlreturn(_T("error in script"), odbc::err_parsing_SQL_statement));
	}
	catch (action_failed &a) {
		tostringstream str;
		str << _T("statement execution returned an error: line ") << a.where.get_position().line << _T(" column ") << a.where.get_position().column;
		sqldiag d;
		d.msg=str.str();
		d.native_error=odbc::err_script_execution_error;
		memcpy(d.state, _T("LWODB"), sizeof(d.state));
		if (m_last_error.ok())
			m_last_error=sqlreturn(SQL_ERROR);
		m_last_error.append_diag(d);
	}
	return last_error();
}

};

};
