#!/usr/bin/python

"""strcasecmp isn't available on win32."""

error_msg = 'Do not use strcasecmp/strncasecmp. Use iequals instead.'

regexp = r'\bstrn?casecmp\s*\('

allowed = [
    'iequals(a, b)',
]

forbidden = [
    'strcasecmp(a, b)',
    '!strncasecmp(a, b, l)',
]
