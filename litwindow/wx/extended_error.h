#ifndef ___extended_error_h__081126104603
#define ___extended_error_h__081126104603
#pragma once

#include <string>
#include <stdexcept>
#include <boost/exception/all.hpp>

namespace litwindow { namespace wx {

    template <typename Char>
    class extended_error
    {
        typedef std::basic_string<Char> string;
    public:
        extended_error(const string &msg, const string &details=string())
            :m_msg(msg), m_details(details), m_was_shown(false)
        {}
        bool was_shown() const { return m_was_shown; }
        void was_shown(bool yes=true) { m_was_shown=true; }

        const string &message() const { return m_msg; }
        void message(const string &msg) { m_msg=msg; }

        const string &details() const { return m_details; }
        void details(const string &d) { m_details=d; }
    protected:
        string m_msg, m_details;
        bool m_was_shown;
    };

    template <typename Char, typename Exception>
    void make_extended_error(const Exception &e, extended_error<Char> &err)
    {
        err.message(e.what());
        err.details(diagnostic_information(e));
    }

    template <typename Char, typename Exception>
    extended_error<Char> make_extended_error(const Exception &e)
    {
        return extended_error<Char>(message(e), diagnostic_information(e));
    }

    inline std::string diagnostic_information(const std::exception &e) { return "std::exception"; }
    inline std::string message(const std::exception &e) { return e.what(); }

    typedef boost::error_info<struct tag_what_info, std::string> what_info;
    typedef boost::error_info<struct tag_filename_info, std::string> filename_info;
    template <>
    inline extended_error<char> make_extended_error(const boost::exception &e)
    {
        std::string const *err=boost::get_error_info<what_info>(e);
        return extended_error<char>(err ? *err : "boost::exception", diagnostic_information(e));
    }
} }

namespace wx1 = litwindow::wx;

#endif // ___extended_error_h__081126104603