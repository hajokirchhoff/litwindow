#pragma once
#include "litwindow/ui/list_mediator.hpp"
#include "litwindow/odbc/statement.h"
#include <string>

namespace litwindow { namespace ui {

	struct odbc_record
	{
		odbc::statement& stmt;
	};

	struct odbc_iterator {};

	class odbc_container_policies
	{
	public:
		using container_type = litwindow::odbc::statement;
		using container_policies_type = odbc_container_policies;
		using value_type = odbc_record;

		using column_descriptor = basic_column_descriptor<value_type>;
		using columns_type = basic_columns_adapter<column_descriptor>;
		using handle_type = int;

		using const_iterator = odbc_iterator;
		using iterator = odbc_iterator;

		void refresh_handles(container_type& cnt) {}
		int get_item_image(container_type& c, const columns_type& columns, size_t row, size_t column) const
		{
			//const handle_type& h(get_row(row));
			int idx = -1;
			//bool rc = columns.render_element_image_at(column, idx, handle_to_value(h));
			return idx;
		}
		ui_string get_item_text(container_type& c, const columns_type& columns, size_t row, size_t column) const
		{
// 			const handle_type& h(get_row(row));
			ui_string rcstring;
			if (columns.at(column).visible()) {
				auto rc = c.fetch_absolute(static_cast<SQLINTEGER>(row) + 1);
				if (rc.no_data()) {
					return _T("nodata");
				}
				else if (rc.fail()) {
					return _("error");
				}
				else {
					columns.render_element_at(column, rcstring, odbc_record{ c });
				}
			}
//			bool rc = columns.render_element_at(column, rcstring, handle_to_value(h));
			return rcstring;
		}
		void set_sort_order(const container_type& c, const columns_type& columns, int column_index, basic_columns_sort_index::sort_type_enum sort_type = basic_columns_sort_index::sort_automatic)
		{
		}
		void set_sort_order(const container_type& c, const columns_type& columns, const basic_columns_sort_index& bcsi)
		{
			set_sort_order(c, columns, bcsi.m_column_index, bcsi.sort_type());
		}
		void clear_sort_order()
		{
		}
		std::vector<basic_columns_sort_index> get_sort_order() const
		{
			return std::vector<basic_columns_sort_index>();
		}
		size_t size(container_type& c) const
		{
			SQLLEN rcount = 0;
			if (c.is_open()) {
				std::wstring stmt = L"SELECT COUNT(*) FROM (" + c.get_statement() + L") AS total";
				odbc::statement count(stmt, c.get_connection());
				count.bind_column(1, rcount);
				auto rc = count.execute();
				auto rc2 = count.fetch();
			}
			return rcount;
		}
	};

	using odbc_container = litwindow::odbc::statement;

	template <>
	class container_policies<odbc_container> :public odbc_container_policies
	{
	public:
		using odbc_record = value_type;
		using column_descriptor = basic_column_descriptor<value_type>;
		using columns_type = basic_columns_adapter<column_descriptor>;
	};

	struct odbc_column
	{
		odbc_column(const std::string& colname) :m_colname(colname) {}
		void operator()(const odbc_record& record, std::wstring& rc)
		{
			if (m_colno < 0)
				m_colno = record.stmt.find_column(litwindow::s2tstring(m_colname));
			record.stmt.get_data_as_string(m_colno, rc);
		}
		std::string m_colname;
		int m_colno = -1;
	};

	template <typename Value>
	struct typed_odbc_column
	{
		typed_odbc_column(const std::string &colname):m_colname(colname) {}
		const Value &operator()(const odbc_record &record)
		{
			if (m_colno < 0)
				m_colno = record.stmt.find_column(litwindow::s2tstring(m_colname));
			record.stmt.get_data(m_colno, m_value);
			return m_value;
		}
		std::string m_colname;
		int m_colno = -1;
		Value m_value;
	};


	template <typename Value>
	void render_as_string(const odbc_record &record, int colno, std::wstring &rc);

	template <typename Value>
	struct typed_odbc_column_formatter
	{
		typed_odbc_column_formatter(const std::string &colname) :m_colname(colname) {}
		void operator()(const odbc_record &record, std::wstring &rc)
		{
			if (m_colno < 0)
				m_colno = record.stmt.find_column(litwindow::s2tstring(m_colname));
			render_as_string<Value>(record, m_colno, rc);
		}
		std::string m_colname;
		int m_colno = -1;
	};

	template <typename Value>
	typed_odbc_column_formatter<Value> odbc_type(const std::string &colname)
	{
		return typed_odbc_column_formatter<Value>(colname);
	}
} }
