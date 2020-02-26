#!/usr/bin/python

"""Do not use std::string::replace to replace a signle character in a
std::string (just assign the desired character to the position)."""

error_msg = """s.replace(pos, 1, "c"); should be s.at(pos) = 'c';."""

strip_comments_and_strings = False
strip_macros = False
regexp = r"""(?x)
replace[ ]*\(
.*,
\s*((0x)?0*1[Uu]?(L|LL|l|ll)?),
\s*(("([^\\]|.[nt"\\])")|('([^\\]|.[nt"\\])'))
\)"""

forbidden = [
    r'replace(pos, 1, "a")',
    r'replace(pos, 1U, "a")',
    r'replace(pos, 1UL, "a")',
    r'replace(pos, 1ULL, "a")',
    r'replace(pos, 1Ul, "a")',
    r'replace(pos, 1Ull, "a")',
    r'replace(pos, 1u, "a")',
    r'replace(pos, 1uL, "a")',
    r'replace(pos, 1uLL, "a")',
    r'replace(pos, 1ul, "a")',
    r'replace(pos, 1ull, "a")',
    r'replace(pos, 01, "a")',
    r'replace(pos, 01U, "a")',
    r'replace(pos, 01UL, "a")',
    r'replace(pos, 01ULL, "a")',
    r'replace(pos, 01Ul, "a")',
    r'replace(pos, 01Ull, "a")',
    r'replace(pos, 01u, "a")',
    r'replace(pos, 01uL, "a")',
    r'replace(pos, 01uLL, "a")',
    r'replace(pos, 01ul, "a")',
    r'replace(pos, 01ull, "a")',
    r'replace(pos, 0x1, "a")',
    r'replace(pos, 0x01, "a")',
    r'replace(pos, 0x01U, "a")',
    r'replace(pos, 0x01UL, "a")',
    r'replace(pos, 0x01ULL, "a")',
    r'replace(pos, 0x01Ul, "a")',
    r'replace(pos, 0x01Ull, "a")',
    r'replace(pos, 0x01u, "a")',
    r'replace(pos, 0x01uL, "a")',
    r'replace(pos, 0x01uLL, "a")',
    r'replace(pos, 0x01ul, "a")',
    r'replace(pos, 0x01ull, "a")',
    r'replace(pos, 1, "\n")',
    r'replace(pos, 1, "\t")',
    r'replace(pos, 1, "\\")',
    r'replace(pos, 1, "\"")',
]

allowed = [
    'replace(pos, 1, "ab");',
    'replace(pos, 2, "a");',
    "at(pos) = 'a';",
    "('.')",
]
