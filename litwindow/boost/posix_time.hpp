#ifndef __LITWINDOW_BOOST_POSIX_TIME_HPP
#define __LITWINDOW_BOOST_POSIX_TIME_HPP

#include <boost/date_time/posix_time/posix_time.hpp>
#include <litwindow/dataadapter.h>

namespace litwindow {
    template <>
    tstring converter<boost::posix_time::ptime>::to_string(const boost::posix_time::ptime &d)
    {
        using namespace boost::posix_time;
        litwindow::tstringstream s;
        boost::date_time::time_facet<ptime, TCHAR> *tfacet(new boost::date_time::time_facet<ptime, TCHAR>(boost::date_time::time_facet<ptime, TCHAR>::iso_time_format_specifier/* _T("%H:%M:%S")*/));
        s.imbue(std::locale(s.getloc(), tfacet));
        s << d;
        return s.str();
    }

    template <>
    size_t converter<boost::posix_time::ptime>::from_string(const tstring &newValue, boost::posix_time::ptime &v)
    {
        using namespace boost::posix_time;
        litwindow::tstringstream s(newValue);
        boost::date_time::time_facet<ptime, TCHAR> *tfacet(new boost::date_time::time_facet<ptime, TCHAR>(boost::date_time::time_facet<ptime, TCHAR>::iso_time_format_specifier/* _T("%H:%M:%S")*/));
        s.imbue(std::locale(s.getloc(), tfacet));
        s >> v;
        return sizeof(v);

    }
}
IMPLEMENT_ADAPTER_TYPE(boost::posix_time::ptime)

#endif