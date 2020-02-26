#!/usr/bin/python

"""
strdup isn't available on win32
"""

error_msg = 'Do not use strdup/strndup. Use std::string instead.'

regexp = r'\bstrn?dup\s*\('

allowed = [
    'y = std::string(x)',

    'y = new char[strlen(x)+1]',
    'std::copy(x, x+strlen(x)+1, y)',
]

forbidden = [
    'y = strdup(x)',
    'y = strndup(x)',
]
