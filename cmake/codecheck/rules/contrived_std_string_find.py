#!/usr/bin/python


"""Do not call std::string::find_first_of or std::string::find with a string of
characters to locate that has the size 1.

Use the version of std::string::find that takes a single character to
locate instead. Same for find_last_of/rfind.
"""

error_msg = "Do not use find(\"a\"), use find('a')."

regexp = r"""(?x)
r?find(_(first|last)_of)?\s*
\(
"([^\\]|(\\[nt\\"]))"[,)]"""

forbidden = [
    r'find_first_of("a")',
    r'find_last_of("a")',
    r'find("a")',
    r'rfind("a")',
    r'find_first_of("\n")',
    r'find_last_of("\n")',
    r'find("\n")',
    r'rfind("\n")',
    r'find_first_of("\t")',
    r'find_last_of("\t")',
    r'find("\t")',
    r'rfind("\t")',
    r'find_first_of("\\")',
    r'find_last_of("\\")',
    r'find("\\")',
    r'rfind("\\")',
    r'find_first_of("\"")',
    r'find_last_of("\"")',
    r'find("\"")',
    r'rfind("\"")',
    r'find_first_of("a", 1)',
    r'find_last_of("a", 1)',
    r'find("a", 1)',
    r'rfind("a", 1)',
]

allowed = [
    r'find("ab")',
    r"find('a')",
    r"rfind('a')",
    r'rfind("ab")',
    r"find('\n')",
    r'find("\nx")',
    r"rfind('\n')",
    r'rfind("\nx")',
    r"find('\t')",
    r'find("\tx")',
    r"rfind('\t')",
    r'rfind("\tx")',
    r"find('\\')",
    r'find("\\x")',
    r"rfind('\\')",
    r'rfind("\\x")',
    r"find('\"')",
    r'find("\"x")',
    r"rfind('\"')",
    r'rfind("\"x")',
    r"find('a', 1)",
    r'find("ab", 1)',
    r"rfind('a', 1)",
    r'rfind("ab", 1)',
]
