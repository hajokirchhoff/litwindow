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

	template <typename OdbcColumn>
	std::string get_column_name(const OdbcColumn &col)
	{
		return col.get_column_name();
	}

	class odbc_column_descriptor :public basic_column_descriptor<odbc_record>
	{
	public:
		odbc_column_descriptor() = default;

		template <typename OdbcColumn>
		odbc_column_descriptor(const tstring &title, int width, const OdbcColumn &col)
			:basic_column_descriptor<odbc_record>(title, width, col), m_sql_column_name(get_column_name(col))
		{}
		template <typename OdbcColumn, typename Formatter>
		odbc_column_descriptor(const tstring &title, int width, const OdbcColumn &col, const Formatter &fmt)
			: basic_column_descriptor<odbc_record>(title, width, col, fmt), m_sql_column_name(get_column_name(col))
		{}

		template <typename Archive>
		void serialize(Archive &ar, const unsigned int version)
		{
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(basic_column_descriptor) & BOOST_SERIALIZATION_NVP(m_sql_column_name);
		}
		std::string m_sql_column_name;
	};

	class odbc_container_policies
	{
	public:
		struct odbc_columns_sort_index :public basic_columns_sort_index
		{
			ui_string m_column_name;
			ui_string ascdesc_as_string() const
			{
				static const ui_string __ASC__ = s2tstring("ASC");
				static const ui_string __DESC__ = s2tstring("DESC");
				return m_sort_ascending == sort_descending ? __DESC__ : __ASC__;
			}
			odbc_columns_sort_index(int idx, sort_type_enum sort, const ui_string &str)
				:basic_columns_sort_index(idx, sort == sort_ascending), m_column_name(str) {}
		};
		struct sorter :public std::vector<odbc_columns_sort_index>
		{
			using sort_type = basic_columns_sort_index::sort_type_enum;
			void push_sort(int column_index, const ui_string &colname, sort_type st = sort_type::sort_automatic)
			{
				auto current = std::find_if(begin(), end(), [=](const auto &i) { return i.m_column_index == column_index; });
				if (current != end()) {
					if (st == sort_type::sort_automatic) {
						if (current->sort_type() == sort_type::sort_ascending && current + 1 == end())
							st = sort_type::sort_descending;
						else
							st = sort_type::sort_ascending;
					}
					erase(current);
				}
				else if (st == sort_type::sort_automatic)
					st = sort_type::sort_ascending;
				emplace_back(column_index, st, colname);
			}
		} m_sorting;

		using container_type = litwindow::odbc::statement;
		using container_policies_type = odbc_container_policies;
		using value_type = odbc_record;

		using column_descriptor = odbc_column_descriptor;
		using columns_type = basic_columns_adapter<column_descriptor>;
		using handle_type = int;

		using const_iterator = odbc_iterator;
		using iterator = odbc_iterator;

		const ui_string __comma__ = litwindow::s2tstring(", ");
		const ui_string __quote__ = litwindow::s2tstring("\"");
		const ui_string __space__ = litwindow::s2tstring(" ");

		ui_string build_where_filter()
		{
			ui_string rc;
			for (auto k = m_sorting.rbegin(); k != m_sorting.rend(); ++k) {
				if (rc.empty() == false)
					rc += __comma__;
				rc += __quote__ + k->m_column_name + __quote__ + __space__ + k->ascdesc_as_string();
			}
			return rc;
		}

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
			if (column_index < columns.size()) {
				m_sorting.push_sort(column_index, s2tstring(columns.at(column_index).m_sql_column_name), sort_type);
			}
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
		//using column_descriptor = basic_column_descriptor<value_type>;
		//using columns_type = basic_columns_adapter<column_descriptor>;
		void refresh_handles(container_type& cnt)
		{
			auto where_filter = build_where_filter();
			if (where_filter.empty() == false) {
				if (m_original_statement.empty())
					m_original_statement = cnt.get_statement();
				cnt.set_statement(L"SELECT * FROM (" + m_original_statement + L") AS sorted_query ORDER BY " + where_filter);
			}
/*
 *
			auto count = cnt.get_column_count();
			tstring name;
			auto cls = cnt.get_column_name(1, name);
*/
			cnt.execute();
		}
		tstring m_original_statement;
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
		std::string get_column_name() const { return m_colname; }
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

		std::string get_column_name() const { return m_colname; }
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
		std::string get_column_name() const { return m_colname; }
		std::string m_colname;
		int m_colno = -1;
	};

	template <typename Value>
	typed_odbc_column_formatter<Value> odbc_type(const std::string &colname)
	{
		return typed_odbc_column_formatter<Value>(colname);
	}
} }