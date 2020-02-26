#!/usr/bin/python -tt

"""Checks that all TRANSLATORS: comments are immediately above the string to be
translated.

Also checks for malformed translators' comment tag. Will catch
permutation typos like "TRASNLATORS".
"""

import re


def evaluate_matches(lines, fn):
    result = []
    translators_comment_started = False
    translators_comment_ended = False
    last_translators_comment = ''
    last_translators_comment_line = 0

    translator_tag = re.compile('.*[*/]\s+[TRANSLATOR]{10}.*')

    for lineno, line in enumerate(lines, 1):
        # Find start of translators' comment
        if translator_tag.match(line):
            translators_comment_started = True
            # Create error for malformed tag - we want consistency here
            if not '/** TRANSLATORS: ' in line:
                result.append((fn, lineno, 'Translators\' comment starting with "' +
                               line.strip() + '" does not start with: "/** TRANSLATORS:"'))
        # Comments can go over multiple lines, so we check for their end
        # separately
        if translators_comment_started:
            last_translators_comment = last_translators_comment + ' ' + line
            if '*/' in line:
                translators_comment_ended = True
                last_translators_comment_line = lineno

        # Check for gettext. This can fail if we have another function name ending with _.
        # Should hopefully fail rarely though, since that's violating our code
        # style.
        if '_("' in line or 'gettext' in line:
            # We have a complete comment, so there should be a gettext call
            # immediately afterwards. Otherwise, xgettext won't find the
            # translators' comment.
            if translators_comment_ended:
                if lineno != last_translators_comment_line + 1:
                    result.append(
                        (fn, lineno, 'Translators\' comment is not directly above its translation:\n' + last_translators_comment.strip()))
            # Reset if there was no complete comment
            translators_comment_started = False
            translators_comment_ended = False
            last_translators_comment = ''
            last_translators_comment_line = 0
    return result


# File this is called on is always called testdir/test.h
forbidden = [
    r"""/** TRANSLATORS: some comment */
    line with some other stuff
    _("translateme");
    """,
    r"""/** TRANSLATORS: some
     comment */
    line with some other stuff
    _("translateme");
    """,
    r"""/** TRANSLATORS: some comment */
    line with some other stuff
    gettext("translateme whatever";
    """,
    r"""/** TRANSLATORS: some
     comment */

    gettext("translateme whatever";
    """,

]

allowed = [
    r"""/** TRANSLATORS: some comment */
    _("translateme");
    """,
    r"""/** TRANSLATORS: some
     comment */
    _("translateme");
    """,
    r"""/** TRANSLATORS: some comment */
    gettext("translateme whatever";
    """,
    r"""/** TRANSLATORS: some
     comment */
    gettext("translateme whatever";
    """,
]
