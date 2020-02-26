#!/usr/bin/python

"""
strcasecmp isn't available on win32
"""

error_msg = 'Do not use strcasecmp/strncasecmp. Use boost::iequals instead.'

regexp = r'\bstrn?casecmp\s*\('

allowed = [
    'boost::iequals(a, b)',
]

forbidden = [
    'strcasecmp(a, b)',
    '!strncasecmp(a, b, l)',
]
