#!/usr/bin/python


error_msg = "Do not use operator synonyms, e.g. use \"&&\" rather than \"and\"."

strip_comments_and_strings = True

regexp = r"""(\b(and|and_eq|bitand|or|or_eq|xor|xor_eq|not|not_eq|compl)\b)"""

forbidden = [
    "if (foo == 0 and bar > 1)",
    "if (foo == 0 bitand bar > 1)",
    "if (foo == 0 and_eq bar > 1)",
    "if (foo == 0 or bar > 1)",
    "if (foo == 0 or_eq bar > 1)",
    "if (foo == 0 xor bar > 1)",
    "if (foo == 0 xor_eq bar > 1)",
    "if (foo == 0 not bar > 1)",
    "if (foo == 0 not_eq bar > 1)",
    "if (foo == 0 compl bar > 1)"
]

allowed = [
    "if (foo == 0 && bar > 1)",
    "if (foo == 0 & bar > 1)",
    "if (foo == 0 &= bar > 1)",
    "if (foo == 0 || bar > 1)",
    "if (foo == 0 |= bar > 1)",
    "if (foo == 0 ^ bar > 1)",
    "if (foo == 0 ^= bar > 1)",
    "if (foo == 0 ! bar > 1)",
    "if (foo == 0 != bar > 1)",
    "if (foo == 0 ~ bar > 1)"
]

