#!/usr/bin/python -tt

"""Checks that all translatable strings that have multiple printf placeholders
have defined those as reversible.

Checks that unordered, ordered and wildcard placeholders aren't mixed up in the same string.

Checks that ngettext singular and plural strings have the same placeholders.

Checks that placeholders are numbered in ascending order.
"""

import re

# Regex to find placeholders
FIND_UNORDERED = re.compile(r'(\%[0-9#*]*\.*[0-9#]*[a-zA-Z]{1,2})')
FIND_ORDERED = re.compile(r'\%[|]{0,1}(\d\$[0-9#*]*\.*[0-9#]*[a-zA-Z]{1,2})')
FIND_WILDCARD = re.compile(r'\%(\d)\%')
CLEAN_WILDCARD_FOR_UNORDERED = re.compile(r'(\%\d\%)')


def FIND_UNORDERED_placeholders(sanitized_entry):
    """We need to remove wildcard matches first, because we have cases like.

    %1%m that match both regex expressions.
    """
    for entry in CLEAN_WILDCARD_FOR_UNORDERED.findall(sanitized_entry):
        sanitized_entry = sanitized_entry.replace(entry, '')
    return FIND_UNORDERED.findall(sanitized_entry)


def check_placeholders(entry):
    """Make sure that a string satisfies our placeholder policy."""
    sanitized_entry = entry.replace('%%', '')
    unordered = FIND_UNORDERED_placeholders(sanitized_entry)
    if len(unordered) > 1:
        return 'Translatable string has multiple sprintf placeholders that are not ordered:'
    else:
        ordered = FIND_ORDERED.findall(sanitized_entry)
        wildcard = FIND_WILDCARD.findall(sanitized_entry)
        if len(unordered) > 0:
            if len(ordered) > 0:
                return 'Translatable string is mixing unordered sprintf placeholders with ordered placeholders:'
            if len(wildcard) > 0:
                return 'Translatable string is mixing unordered sprintf placeholders with wildcard placeholders:'
        if len(ordered) > 0 and len(wildcard) > 0:
            return 'Translatable string is mixing ordered sprintf placeholders with wildcard placeholders:'
        if len(ordered) > 0:
            for entryno, placeholder in enumerate(ordered, 1):
                if str(entryno) != placeholder[:placeholder.find('$')]:
                    return 'Translatable string has an ordered sprintf placeholder "' + placeholder + '" in position ' + str(entryno) + " - the numbers don't match:"
        if len(wildcard) > 0:
            for entryno, placeholder in enumerate(wildcard, 1):
                if str(entryno) != placeholder:
                    return 'Translatable string has an ordered wildcard placeholder "' + placeholder + '" in position ' + str(entryno) + " - the numbers don't match:"
    return ''


def compare_placeholders(entry1, entry2):
    """An Ngettext string must have the same placeholders in its singular and
    plural strings."""
    sanitized_entry1 = entry1.replace('%%', '')
    sanitized_entry2 = entry2.replace('%%', '')

    # There is interaction between wildcard and unordered, so wildcard has to come first.
    placeholders1 = FIND_WILDCARD.findall(sanitized_entry1)
    placeholders2 = FIND_WILDCARD.findall(sanitized_entry2)
    if len(placeholders1) == 0 and len(placeholders2) == 0:
        placeholders1 = FIND_ORDERED.findall(sanitized_entry1)
        placeholders2 = FIND_ORDERED.findall(sanitized_entry2)
        if len(placeholders1) == 0 and len(placeholders2) == 0:
            placeholders1 = FIND_UNORDERED.findall(sanitized_entry1)
            placeholders2 = FIND_UNORDERED.findall(sanitized_entry2)

    if len(placeholders1) != len(placeholders2):
        return 'Ngettext string has mismatching number of placeholders. Singular has ' + str(len(placeholders1)) + ' placeholder(s) and plural has ' + str(len(placeholders2)) + ' placeholder(s):'

    for entryno, placeholder in enumerate(placeholders1, 0):
        if placeholder != placeholders2[entryno]:
            return 'Ngettext string has mismatching placeholders "' + placeholder + '" and "' + placeholders2[entryno] + '" in position ' + str(entryno + 1) + ':'

    return ''


def find_line(entry, lines):
    """Find the line for the error entry.

    Grabs the first match, so this is imprecise for multiple matches
    """
    checkme = entry.split('\n')
    for lineno, line in enumerate(lines, 0):
        if checkme[0] in line:
            is_match = True
            for rowno, row in enumerate(checkme, 0):
                if (lineno + rowno) < len(lines):
                    if not checkme[rowno] in lines[lineno + rowno]:
                        is_match = False
                        break
            if is_match:
                return lineno + 1
    return 0


def evaluate_matches(lines, fn):
    """Main check."""
    result = []

    searchme = ''.join(lines).strip()

    single_strings = []
    string_pairs = []

    # Simple gettext
    # DOTALL makes . match newlines
    matches = re.findall(r"_\(\"(.*?)\"\)", searchme, re.DOTALL)
    for match in matches:
        if '%' in match:
            single_strings.append(match)

    matches = re.findall(r"\Wgettext\(\"(.*?)\"\)", searchme, re.DOTALL)
    for match in matches:
        if '%' in match:
            single_strings.append(match)

    # pgettext
    matches = re.findall(
        r"\Wpgettext\(\".*?\",.+?\"(.*?)\"\)", searchme, re.DOTALL)
    for match in matches:
        if '%' in match:
            single_strings.append(match)

    # ngettext
    matches = re.findall(
        r"ngettext\(\"(.*?)\",.+?\"(.*?)\".+?\w+?\)", searchme, re.DOTALL)
    for match in matches:
        if '%' in match[0] or '%' in match[1]:
            string_pairs.append(match)

    # npgettext
    matches = re.findall(
        r"npgettext\(\".*?\",.+?(.*?)\",.+?\"(.*?)\".+?\w+?\)", searchme, re.DOTALL)
    for match in matches:
        if '%' in match[0] or '%' in match[1]:
            string_pairs.append(match)

    for entry in single_strings:
        check_result = check_placeholders(entry)
        if len(check_result) > 0:
            result.append((fn, find_line(entry, lines),
                           check_result + '\n' + entry))

    for checkme in string_pairs:
        for entry in checkme:
            check_result = check_placeholders(entry)
            if len(check_result) > 0:
                result.append((fn, find_line(
                    checkme[0], lines), check_result + '\n' + checkme[0] + ' - ' + checkme[1]))

        check_result = compare_placeholders(checkme[0], checkme[1])
        if len(check_result) > 0:
            result.append((fn, find_line(
                checkme[0], lines), check_result + '\n' + checkme[0] + ' - ' + checkme[1]))

    return result


# File this is called on is always called testdir/test.h
forbidden = [
    # Unordered with multiple
    '_("One %d and another %s")',

    # Mixed Wildcard + Ordered
    '_("One %1$i and another %2%")',

    # Mixed Wildcard + Unordered
    '_("One %i and another %2%")',

    # Mixed Ordered + Unordered
    '_("One %i and another %2$d")',

    # Wrong order
    '_("One %2% and another %1%")',
    '_("One %2$lu and another %1$lu")',
]

allowed = [
    # Wildcard followed by letter
    'npgettext("foo", "%1%d", "%1%d", value)',
    'ngettext("%1%m", "%1%m", value)',
    'gettext("%1%s")',
    '_("%1%s")',
    # Wildcard
    'npgettext("foo", "%1%", "%1%", value)',
    'ngettext("%1%", "%1%", value)',
    'gettext("%1%")',
    '_("%1%")',
    # Wildcard with percent
    'npgettext("foo", "%1%%%", "%1%%%", value)',
    'ngettext("%1%%%", "%1%%%", value)',
    'gettext("%1%%%")',
    '_("%1%%%")',
    # Wildcard with multiple
    'npgettext("foo", "One %1% and another %2%", "Many %1% and another %2%", value,)',
    'ngettext("One %1% and another %2%", "Many %1% and another %2%", value)',
    'gettext("One %1% and another %2%")',
    '_("One %1% and another %2%")',
    # Ordered
    'npgettext("foo", "%1$d", "%1$d", value)',
    'ngettext("%1$d", "%1$d", value)',
    'gettext("%1%s")',
    '_("%1$s")',
    # Ordered with percent
    'npgettext("foo", "%1$d%%", "%1$d%%", value)',
    'ngettext("%1$d%%", "%1$d%%", value)',
    'gettext("%1$s%%")',
    '_("%1%s%%")',
    # Ordered with multiple
    'npgettext("foo", "One %1$d and another %2$s", "Many %1$d and another %2$s", value,)',
    'ngettext("One %1$lu and another %2$s", "Many %1$lu and another %2$s", value)',
    'gettext("One %1$d and another %2$s")',
    '_("One %1$d and another %2$s")',
    # Unordered
    'npgettext("foo", "%d", "%d", value)',
    'ngettext("%d", "%d", value)',
    'gettext("%s")',
    '_("%1$s")',
    # Unordered with percent
    'npgettext("foo", "%d%%", "%d%%", value)',
    'ngettext("%lu%%", "%lu%%", value)',
    'gettext("%s%%")',
    '_("%s%%")',
]
