/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: tstring.cpp,v 1.2 2006/03/29 13:51:42 Hajo Kirchhoff Exp $
*/
#include "stdafx.h"
#include "litwindow/tstring.hpp"
#define new DEBUG_NEW

#ifdef USE_INLINING
#define INLINE inline
#else
#define INLINE
#endif

namespace litwindow {

	template <typename Splitter>
	Splitter split_string(const tstring &in, Splitter splitter, const tstring &delimiters, const tstring &quotes, const tstring &equal_sign)
	{
		const TCHAR *p=in.c_str();
		TCHAR end_delimiter=0;
		 TCHAR current_quote_char=0;
		const TCHAR *begin_part=0;
		const TCHAR *end_part=0;
		bool uses_double_quote_escape=false;
		bool uses_backslash_escape=false;
		bool allow_backslash_escape=false;
		enum {
			scan_for_left_begin,
			scan_for_left_end,
			scan_for_equal_sign,
			scan_for_right_begin,
			scan_for_right_end,
			scan_for_delimiter,
			scan_error,
			scan_end
		} state=equal_sign.length()>0 ? scan_for_left_begin : scan_for_right_begin;
		tstring left_value, right_value;
		while (*p || state==scan_for_right_end) {
			switch (state) {
				case scan_for_left_begin:
				case scan_for_right_begin:
					if (*p!=_T(' ')) {
						if (quotes.find(*p)!=tstring::npos) {
							// quoted part begins
							current_quote_char=*p;
							begin_part=p+1;
							allow_backslash_escape=true;
							switch (current_quote_char) 
							{
							case _T('{'): 
								end_delimiter=_T('}'); 
								break;
							case _T('['): 
								end_delimiter=_T(']'); 
								break;
							case _T('<'): 
								end_delimiter=_T('>'); 
								break;
							default:
								end_delimiter=current_quote_char;
								allow_backslash_escape=false;
								break;
							}
						} else {
							// non-quoted part begins
							current_quote_char=0;
							begin_part=p;
						}
						end_part=0;
						if (state==scan_for_left_begin)
							state=scan_for_left_end;
						else
							state=scan_for_right_end;
					}
					break;
				case scan_for_left_end:
				case scan_for_right_end:
					{
						if (end_delimiter) {
							// inside a quoted part of the string
							if (p[0]==current_quote_char && p[1]==current_quote_char) {
								++p;
								uses_double_quote_escape=true;
							} else if (allow_backslash_escape && *p==_T('\\')) {
								++p;
								uses_backslash_escape=true;
							} else if (*p==end_delimiter) {
								// end of quoted part
								end_part=p;
								++p;	// eat end delimiter
							} else if (*p==0) {
								--p;
								state=scan_error;	// missing right delimiter
							} // else still inside quoted part
						} else if (*p==0 || *p==_T(' ') || state==scan_for_left_end && equal_sign.find(*p)!=tstring::npos || state==scan_for_right_end && delimiters.find(*p)!=tstring::npos) {
							end_part=p;
						}
						if (end_part) {
							--p; 	// put back current character for next phase
							right_value=tstring(begin_part, end_part);
							if (uses_double_quote_escape || uses_backslash_escape) {
								for (tstring::iterator i=right_value.begin(); i!=right_value.end(); ++i) {
									if ((uses_double_quote_escape && *i==current_quote_char) || (uses_backslash_escape && *i==_T('\\')))
										i=right_value.erase(i);
								}
							}
							if (state==scan_for_left_end) {
								left_value=right_value;
								state=scan_for_equal_sign;
							} else {
								splitter(left_value, right_value);
								state=p[1] ? scan_for_delimiter : scan_end;
							}
						}
					} break;
				case scan_for_equal_sign:
					if (*p!=_T(' ')) {
						if (equal_sign.find(*p)!=tstring::npos) {
							state=scan_for_right_begin;
						} else
							throw runtime_error("unexpected character after left_value");
					}
					break;
				case scan_for_delimiter:
					if (delimiters.find(*p)!=tstring::npos) {
						state=equal_sign.length() > 0 ? scan_for_left_begin : scan_for_right_begin;
					} else if (*p!=_T(' ')) {
						throw runtime_error("unexpected character after right_value");
					}
					break;
			}
			++p;
		}
		if (state==scan_for_left_begin || equal_sign.length()==0 && state==scan_for_right_begin)
			state=scan_end;
		if (state!=scan_end) {
			throw runtime_error("unterminated left_value=right_value sequence");
		}
		return splitter;
	}


	struct string_insert_map
	{
		std::map<tstring, tstring> *to_insert;
		void operator()(const tstring &left, const tstring &right) const
		{
			if (to_insert->insert(make_pair(left, right)).second==false)
				throw runtime_error("duplicate values in split_string");
		}
	};
	std::map<tstring, tstring> split_string(const tstring &in, const tstring &delimiters, const tstring &quotes, const tstring &equal_sign)
	{
		std::map<tstring, tstring> rc;
		string_insert_map sim;
		sim.to_insert=&rc;
		split_string(in, sim, delimiters, quotes, equal_sign);
		return rc;
	}
	struct string_push_back_vector
	{
		std::vector<tstring> *to_insert;
		void operator()(const tstring &, const tstring &right) const
		{
			to_insert->push_back(right);
		}
	};
	void split_string(std::vector<tstring> &rc, const tstring &in, const tstring &delimiters, const tstring &quotes)
	{
		string_push_back_vector action;
		action.to_insert=&rc;
		split_string(in, action, delimiters, quotes, tstring());
	}

	//std::map<tstring, tstring> split_string(const tstring &in, const tstring &delimiters, const tstring &quotes, const tstring &equal_sign)
	//{
	//	std::map<tstring, tstring> rc;
	//	const TCHAR *p=in.c_str();
	//	TCHAR end_delimiter=0;
	//	 TCHAR current_quote_char=0;
	//	const TCHAR *begin_part=0;
	//	const TCHAR *end_part=0;
	//	bool uses_double_quote_escape=false;
	//	bool uses_backslash_escape=false;
	//	bool allow_backslash_escape=false;
	//	enum {
	//		scan_for_left_begin,
	//		scan_for_left_end,
	//		scan_for_equal_sign,
	//		scan_for_right_begin,
	//		scan_for_right_end,
	//		scan_for_delimiter
	//	} state=scan_for_left_begin;
	//	tstring left_value, right_value;
	//	while (*p) {
	//		switch (state) {
	//			case scan_for_left_begin:
	//			case scan_for_right_begin:
	//				if (*p!=_T(' ')) {
	//					if (quotes.find(*p)!=tstring::npos) {
	//						// quoted part begins
	//						current_quote_char=*p;
	//						begin_part=p+1;
	//						allow_backslash_escape=true;
	//						switch (current_quote_char) 
	//						{
	//						case _T('{'): 
	//							end_delimiter=_T('}'); 
	//							break;
	//						case _T('['): 
	//							end_delimiter=_T(']'); 
	//							break;
	//						case _T('<'): 
	//							end_delimiter=_T('>'); 
	//							break;
	//						default:
	//							end_delimiter=current_quote_char;
	//							allow_backslash_escape=false;
	//							break;
	//						}
	//					} else {
	//						// non-quoted part begins
	//						current_quote_char=0;
	//						begin_part=p;
	//					}
	//					end_part=0;
	//					if (state==scan_for_left_begin)
	//						state=scan_for_left_end;
	//					else
	//						state=scan_for_right_end;
	//				}
	//				break;
	//			case scan_for_left_end:
	//			case scan_for_right_end:
	//				{
	//					if (end_delimiter) {
	//						// inside a quoted part of the string
	//						if (p[0]==current_quote_char && p[1]==current_quote_char) {
	//							++p;
	//							uses_double_quote_escape=true;
	//						} else if (allow_backslash_escape && *p==_T('\\')) {
	//							++p;
	//							uses_backslash_escape=true;
	//						} else if (*p==end_delimiter) {
	//							// end of quoted part
	//							end_part=--p;
	//						} // else still inside quoted part
	//					} else if (*p==_T(' ') || state==scan_for_left_end && equal_sign.find(*p)!=tstring::npos || state==scan_for_right_end && delimiters.find(*p)!=tstring::npos) {
	//						end_part=--p;	// put back current character for next phase
	//					}
	//					if (end_part) {
	//						right_value=tstring(begin_part, end_part);
	//						if (uses_double_quote_escape || uses_backslash_escape) {
	//							for (tstring::iterator i=right_value.begin(); i!=right_value.end(); ++i) {
	//								if ((uses_double_quote_escape && *i==current_quote_char) || (uses_backslash_escape && *i==_T('\\')))
	//									i=right_value.erase(i);
	//							}
	//						}
	//						if (state==scan_for_left_end) {
	//							left_value=right_value;
	//							state=scan_for_equal_sign;
	//						} else {
	//							if (rc.insert(make_pair(left_value, right_value)).second==false) {
	//								throw runtime_error("duplicate value");
	//							}
	//							state=scan_for_delimiter;
	//						}
	//					}
	//				} break;
	//			case scan_for_equal_sign:
	//				if (*p!=_T(' ')) {
	//					if (equal_sign.find(*p)!=tstring::npos) {
	//						state=scan_for_right_begin;
	//					} else
	//						throw runtime_error("unexpected character after left_value");
	//				}
	//				break;
	//			case scan_for_delimiter:
	//				if (delimiters.find(*p)!=tstring::npos) {
	//					state=scan_for_left_begin;
	//				} else if (*p!=_T(' ')) {
	//					throw runtime_error("unexpected character after right_value");
	//				}
	//				break;
	//		}
	//		++p;
	//	}
	//	if (state!=scan_for_left_begin) {
	//		throw runtime_error("unterminated left_value=right_value sequence");
	//	}
	//	return rc;
	//}

};

