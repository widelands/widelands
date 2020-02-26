#!/usr/bin/python


error_msg="Do not inherit from boost::noncopyable. Instead #include \"base/macros.h\" and add \"DISALLOW_COPY_AND_ASSIGN(<Objectname>);\" as the last entry in the private section."

regexp=r"""(^#include.*boost.noncopyable\.hpp|noncopyable)"""

forbidden = [
    "#include <boost/noncopyable.hpp>",
    "noncopyable"
]

allowed = [
    "DISALLOW_COPY_AND_ASSIGN",
]
