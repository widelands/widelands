#!/usr/bin/python


"""
This catches functions that return a value as const, which makes no sense.
Unfortunately it only catches virtual functions.
"""

error_msg = "Do not return values as 'const'. This does not make sense."

regexp="""^\s*virtual +(?:const +(?:[_a-zA-Z][_a-zA-Z0-9]*::)*[_a-zA-Z][_a-zA-Z0-9]*|(?:[_a-zA-Z][_a-zA-Z0-9]*::)*[_a-zA-Z][_a-zA-Z0-9]* +const) *[_a-zA-Z][_a-zA-Z0-9]* *\("""

forbidden = [
    'virtual const My::integer f();',
    'virtual bool const gh() {',
]

allowed = [
    'virtual My:integer f();',
    'virtual bool gh() {',
]
