#!/usr/bin/python -tt

# QT Creator likes to add __cxx11 to the namsepace when autocompleting
# std::string, which will break clang builds.
def does_include_cxx11(lines, fn):
    for lineno, line in enumerate(lines, 1):
        if "std::__cxx11" in line:
            return [ (fn, lineno,
                "Do not use the std::__cxx11 namespace.") ]
    return []

evaluate_matches = does_include_cxx11


#################
# ALLOWED TESTS #
#################
allowed = [
"""std::string
""",
]


###################
# FORBIDDEN TESTS #
###################
forbidden = [
"""std::__cxx11::string
""",
]
