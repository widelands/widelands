#!/usr/bin/python

"""This catches std::string constants that are passed by value.

They should be passed by reference to avoid needless
construction/destruction.
"""

error_msg = 'const std::string must not be passed by value. Pass by reference!'

regexp = r"""[\( ](?:const +std::string|std::string +const)(?: +[_a-zA-Z][_a-zA-Z0-9]*)?(?: *=.*)?(?:,(?: |$)|\))"""

forbidden = [
    '(std::string const,',
    ' std::string const abc,',
    ' std::string const abc = "abc",',
    ' std::string const = "abc",',
    ' std::string const)',
    '(const std::string,',
    ' const std::string abc,',
    ' const std::string abc = "abc",',
    ' const std::string = "abc",',
    ' const std::string)',
]

allowed = [
    '(std::string const &,'
    ' std::string const & abc,',
    ' std::string const & = "abc",',
    ' std::string const &)',
    '(const std::string &,',
    ' const std::string & abc,',
    ' const std::string & = "abc",',
    ' const std::string &)',
    ' std::string const abc;',
    ' std::string const abc = "abc";',
    ' std::string const abc("abc");',
    ' const std::string abc;',
    ' const std::string abc = "abc";',
    ' const std::string abc("abc");',
]
