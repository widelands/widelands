#!/usr/bin/python


"""
Opening braces need to be at the start of a line, or be preceded by a space.
"""

error_msg = "A space is mandatory before '{' unless it is part of an expression or at the start of a line."

strip_comments_and_strings = True
regexp = r"""[^\w\s\t\[({<>]{"""


forbidden = [
    "if (something){",
]

allowed = [
    "if (something) {",
    "{",
    "\t{",
    " {",
    "container.at({a, tuple})",
    "container[{a, tuple}]",
    "<{a, tuple}>",
    "InitDataType{a, tuple}",
    "{{a, nested}, tuple}",
    "std::map<std::string, std::string>{a, tuple}",
]
