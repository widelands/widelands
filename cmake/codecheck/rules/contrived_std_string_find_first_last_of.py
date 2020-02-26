#!/usr/bin/python


"""
Do not call std::string::find_first_of with a single character to
find. Use the version of std::string::find that takes a single
character to locate instead.  Same for find_last_of/rfind.
"""

error_msg = "Do not use find_first_of('a'), use find('a') " \
    "(same for find_last_of/rfind)."

regexp = r"""(?x)
find_(first|last)_of\s*
\(
'([^\\]|(\\[nt'\\]))'[,)]"""

forbidden = [
    r"find_first_of('a')",
    r"find_first_of('\n')",
    r"find_first_of('\\')",
    r"find_first_of('a', 1)",
    r"find_last_of('a')",
    r"find_last_of('\n')",
    r"find_last_of('\\')",
    r"find_last_of('a', 1)",
]

allowed = [
    r"""find_first_of("hallo")""",
    r"find('a')",
    r"rfind('a')",
    r"find('\n')",
    r"rfind('\n')",
    r"find('\t')",
    r"rfind('\t')",
    r"find('\\')",
    r"rfind('\\')",
    r"find('\"')",
    r"rfind('\"')",
    r"find('a', 1)",
    r"rfind('a', 1)",
]
