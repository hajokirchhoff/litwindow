/* 
* Copyright 2004-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: rapidUI_parser.cpp,v 1.5 2007/10/01 14:36:37 Hajo Kirchhoff Exp $
*/
#include "Stdwx.h"
#include "litwindow/algorithm.h"
#include "litwindow/logging.h"
#include "litwindow/constraints.h"
#include "litwindow/wx/rapidUI.h"
#include "litwindow/check.hpp"
#include <boost/spirit.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace litwindow {

	using namespace boost;

	struct comment_parser:public spirit::grammar<comment_parser>
	{
		template <typename ScannerT>
		struct definition {
			definition(comment_parser const &self)
			{
				using namespace spirit;
				oneline_comment =
					str_p(_T("//")) >>
					lexeme_d[*(~ chset_p(_T("\r\n")))];
				multiline_comment =
					confix_p(str_p(_T("/*")), *anychar_p, str_p(_T("*/")));
				comment =
					oneline_comment | multiline_comment | space_p;
			}
			spirit::rule<ScannerT> const &start() const { return comment; }
			spirit::rule<ScannerT> multiline_comment, oneline_comment, comment;
		};
	};

	comment_parser const comment_p=comment_parser();

	struct rules_grammar:public spirit::grammar<rules_grammar>
	{
		enum assignment_type {
			assign_oneway,
			assign_twoway
		};
		static const struct assignment_operator:public spirit::symbols<assignment_type, TCHAR> 
		{
			assignment_operator()
			{
				add
					(_T(":="), assign_oneway)
					(_T("="), assign_oneway)
					(_T(":=:"), assign_twoway)
					(_T("=="), assign_twoway)
					;
			}
		} assignment_operator_p;
		template <typename ScannerT>
		struct definition {
			definition(rules_grammar const &self)
			{
				using namespace spirit;
				identifier = (alpha_p | _T('_')) >> *(alnum_p | _T('_'));
				fully_qualified_identifier = identifier % (str_p(_T('.')) | _T("::"));
				assign = (
					fully_qualified_identifier[assign_a(left)] >> 
					(assignment_operator_p/*(str_p(_T('=')) | str_p(_T("=="))*/[assign_a(s_assignment)]) >>
					fully_qualified_identifier[assign_a(right)]
				)
					[boost::bind(self.assign_action, boost::ref(left), boost::ref(right), boost::ref(s_assignment))]
				;
				statement = 
					assign | eps_p;
				statement_list = statement % _T(';');
			};
			spirit::rule<ScannerT> const &start() const { return statement_list; }
			spirit::rule<ScannerT> statement_list, statement, assign, identifier, fully_qualified_identifier;
			tstring left, right;
			assignment_type s_assignment;
		};
		boost::function<void(const tstring& left, const tstring& right, assignment_type assign_type)> assign_action;
	};

	const rules_grammar::assignment_operator rules_grammar::assignment_operator_p;

	struct rules_parser:public rules_grammar
	{
		rules_parser(RapidUI *target, const tstring &default_group)
			:m_target(target), m_default_group(default_group)
		{
			assign_action=boost::bind(&rules_parser::assign, *this, _1, _2, _3);
		}
		void assign(const tstring &left, const tstring &right, assignment_type assign_type)
		{
			litwindow::context_t c("rules: assign");
			if (assign_type==assign_twoway)
				m_target->AssignTwoWay(left, right);
			else
				m_target->Assign(m_target->GetAccessor(left), m_target->GetAccessor(right), m_default_group);
		}
		RapidUI *m_target;
		tstring m_default_group;
	};

	void RapidUI::add_rules(const tstring &rules_list, const tstring &to_group, const tstring &source_url)
	{
		litwindow::context_t c("RapidUI::add_rules");
		c["source_url"]=t2string(source_url);
		rules_parser language(this, to_group);
		spirit::parse_info<TCHAR const*> info=spirit::parse(rules_list.c_str(), rules_list.c_str()+rules_list.length(), language >> spirit::end_p, comment_p);
		if (info.full==false) {
			c["error location"]=t2string(info.stop);
			lw_err() << source_url << _T(": rules parser syntax error: ") << tstring(info.stop, 20) << endl;
			throw lwbase_error("syntax error");
		}
	}

};
