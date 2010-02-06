#!/usr/bin/python


"""
This catches some functions that are declared to have no parameters with the
syntax "some_function(void)".  
"""

regexp = """\( *(const)* *void *\)"""
error_msg = "void f(void) is invalid. Use void f() instead!"

forbidden = [
    'void f(void)',
    'void f(const void)',
]

allowed = [
    "void f()",
]



