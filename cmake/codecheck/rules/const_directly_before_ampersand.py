#!/usr/bin/python

"""
Use const Klass &, not Klass const &
"""

error_msg="Use const Klass&, not Klass const&."

regexp=r"""const\s*&"""

forbidden = [
	"(std::string const & fontname, int32_t const fontsize,"
]

allowed = [
    "(const std::string & fontname, int32_t const fontsize,"
]
