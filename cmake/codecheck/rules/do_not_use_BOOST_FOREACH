#!/usr/bin/python

"""
Prefer DEBUG.
"""

error_msg="Do not use BOOST_FOREACH. Instead use C++11 for each loops."

regexp=r"""(^#include.*boost.foreach\.hpp|BOOST_FOREACH)"""

forbidden = [
    "#include <boost/foreach.hpp>",
    "BOOST_FOREACH"
]

allowed = [
]
