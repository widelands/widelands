#!/usr/bin/python -tt

"""This proggy parses given files for occurences of translatable strings if
some are found they are output in a xgettext style file."""

import sys
import os

from collections import defaultdict


class occurences:
    def __init__(self, file, line):
        self.line = line
        self.file = file


class trans_string:
    def __init__(self):
        self.occurences = []
        self.str = ''


def is_multiline(str):
    l = str.find('""')
    if (l == -1):
        return False
    return True


def firstl(str, what):
    for c in what:
        index = str.find(c)
        if (index != -1):
            return index
    return -1


def firstr(str, what):
    for c in what:
        index = str.rfind(c)
        if (index != -1):
            return index
    return -1


def append_string(known_strings, array, string):
    if string.str in known_strings:
        i = known_strings[string.str]
        array[i].occurences += string.occurences
    else:
        array.append(string)
        known_strings[string.str] = len(array) - 1


def _escape_pot_string(string):
    return string.replace('\\', '\\\\').replace('"', '\\"')


def _format_msgid(tag, string, output):
    # there was a bug in this code that would never output single line
    # msg_ids. I decided not to fix it, since that wuold change a ton
    # of pot files and maybe msg_ids.
    # if not string.count('\n'): <== this was s.count
    # s += 'msgid "%s"\n' % string
    # else:
    string = _escape_pot_string(string)
    output.append('%s ""' % tag)
    lines = string.split('\n')
    for line in lines[:-1]:
        output.append('"%s\\n"' % line)
    output.append('"%s"' % lines[-1])
    return output


class Conf_GetText(object):
    def __init__(self):
        self.translatable_strings = []

    @property
    def found_something_to_translate(self):
        return len(self.translatable_strings) > 0

    def parse(self, files):
        known_strings = {}
        curstr = 0
        multiline = 0
        for file in files:
            lines = open(file, 'r').readlines()
            for i in range(0, len(lines)):
                line = lines[i].rstrip('\n')
                linenr = i + 1
                curstr = 0

                if multiline and len(line) and line[0] == '_':
                    line = line[1:]
                    rindex = line.rfind('""')
                    if rindex == -1 or line[:2] == '""':
                        line = line.strip()
                        line = line.strip('"')
                        curstr = trans_string()
                        curstr.str = line
                        curstr.occurences.append(occurences(file, linenr))
                        append_string(
                            known_strings, self.translatable_strings, curstr)
                        continue
                    else:
                        line = line[:(rindex + 1)]
                        line = line.strip()
                        line = line.strip('"')
                        curstr = trans_string()
                        curstr.str = line
                        curstr.occurences.append(occurences(file, linenr))
                        append_string(
                            known_strings, self.translatable_strings, curstr)
                        multiline = False
                        continue

                index = line.find('=_')
                if (index > 0):
                    curstr = trans_string()
                    curstr.occurences.append(occurences(file, linenr))
                    restline = line[index + 2:]
                    multiline = is_multiline(restline)
                    if multiline:  # means exactly one "
                        index = firstl(restline, '"\'')
                        restline = restline[index + 1:]
                        restline = restline.strip()
                        restline = restline.strip('"')
                        curstr.str += '"' + restline + '"\n'
                        continue
                    # Is not multiline
                    # If there are ' or " its easy
                    l = firstl(restline, '\"')
                    r = firstr(restline[l + 1:], '\"')
                    if (l != -1 and r != -1):
                        restline = restline[l + 1:]
                        restline = restline[:r]
                    else:
                        # Remove comments
                        rindex = max(restline.rfind('#'), restline.rfind('//'))
                        if rindex != -1:
                            restline = restline[:rindex]
                        # And strip
                        restline = restline.strip()
                    curstr.str = restline
                    append_string(
                        known_strings, self.translatable_strings, curstr)

    def toString(self):
        lines = []
        for i in self.translatable_strings:
            for occ in i.occurences:
                lines.append('#: %s:%i' %
                             (os.path.normpath(occ.file), occ.line))

            _format_msgid('msgid', i.str, lines)
            lines.extend(['msgstr ""', ''])
        return ('\n'.join(lines))
