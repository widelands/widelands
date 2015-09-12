#!/usr/bin/env python
# encoding: utf-8

from collections import defaultdict, namedtuple
import itertools
import os
import re

from confgettext import head

Token = namedtuple('Token', ['type', 'data'])

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

class ParsingNode(list):

    def __init__(self, parent=None):
        self.parent = parent
        self.type = 'NODE'

    def __str__(self):
        return 'N%s' % list.__str__(self)


class LuaParser(object):

    def __init__(self):
        self.scanner = re.Scanner([
            (r"\-\-( TRANSLATORS)?", self._Tcomment),
            (r"\(", self._Tleft),
            (r"\)", self._Tright),
            (r"\.\.", self._Tconcat),
            (r"(?<!\\)[\"']", self._Ttoggle_string),
            (r"\[\[", self._Topen_multiline_string),
            (r"\]\]", self._Tclose_multiline_string),
            (r"\b_\b", self._Ttranslate_next_string),
            (r"[^\d\W]\w*", self._Tidentifier),
            (r"\n", self._Tnewline),
            (r"[ \t]+", self._Twhitespace),
            (r".", self._Tidentifier),
        ], re.MULTILINE | re.DOTALL)
        self.in_string = False
        self.current_line = 1
        self.current_string = ''
        self.result = ParsingNode()
        self.in_comment = False
        self.translator_comment = ''
        self.current = self.result

    def parse(self, content):
        self.scanner.scan(content)

        def _recurse_lists(original_list, func):
            def _internal(l):
                new_l = ParsingNode()
                for entry in l:
                    if isinstance(entry, list):
                        entry = _recurse_lists(entry, func)
                    new_l.append(entry)
                return func(new_l)
            return _internal(original_list)

        def _combine_concatenated_strings(l):
            for i in range(len(l)):
                if i > 0 and l[i].type == '..':
                    if l[i - 1].type != 'STRING' or l[i + 1].type != 'STRING':
                        continue
                    # Do not concatenate translated and untranslated string.
                    if i - 2 >= 0 and l[i - 2].type == '_':
                        continue
                    first_string, line = l[i - 1].data
                    second_string, unused = l[i + 1].data
                    l[i - 1:i +
                        2] = [Token('STRING', (first_string + second_string, line))]
                    return _combine_concatenated_strings(l)
            return l

        translatable_items = []
        translator_comment_lines = []

        def _find_translatable_strings(l):
            for i in range(len(l)):
                if isinstance(l[i], list):
                    _find_translatable_strings(l[i])
                    continue

                append_data = None
                if l[i].type == 'TRANSLATOR_COMMENT':
                    translator_comment_lines.append(l[i].data[0])

                if l[i].type == '_':
                    if l[i + 1].type == 'NODE':
                        msgid, line = l[i + 1][0].data
                    else:
                        msgid, line = l[i + 1].data
                    append_data = {
                        'type': 'gettext',
                        'msgid': msgid,
                        'line': line,
                    }

                if l[i].type == 'pgettext':
                    assert l[i + 1].type == 'NODE'
                    assert len(l[i + 1]) == 3
                    line = l[i + 1][0].data[1]
                    msgctxt = l[i + 1][0].data[0]
                    msgid = l[i + 1][2].data[0]
                    append_data = {
                        'type': 'pgettext',
                        'msgid': msgid,
                        'msgctxt': msgctxt,
                        'line': line,
                    }

                if l[i].type == 'ngettext':
                    assert l[i + 1].type == 'NODE'
                    assert len(l[i + 1]) >= 4
                    msgid, line = l[i + 1][0].data
                    msgid_plural = l[i + 1][2].data[0]
                    append_data = {
                        'type': 'ngettext',
                        'msgid': msgid,
                        'msgid_plural': msgid_plural,
                        'line': line,
                    }

                if append_data:
                    comment = '\n'.join(translator_comment_lines).strip()
                    # Empty the list, but keep the reference the same, so that
                    # all recursed methods still use the same object.
                    while translator_comment_lines:
                        translator_comment_lines.pop()
                    if comment:
                        append_data['translator_comment'] = comment
                    translatable_items.append(append_data)
            return l

        def _remove_empties(l):
            rv = ParsingNode()
            for a in l:
                if isinstance(a, list) and not a:
                    continue
                rv.append(a)
            return rv

        self.result = _recurse_lists(self.result, _remove_empties)
        self.result = _recurse_lists(
            self.result, _combine_concatenated_strings)

        _find_translatable_strings(self.result)
        return translatable_items

    def _skip_token(self, scanner, token):
        if self.in_string:
            self.current_string += token
        elif self.translator_comment:
            self.translator_comment += token

    def _string_done(self):
        text = eval('str(""" %s """)' % self.current_string)[1:-1]
        self.current.append(Token('STRING', (text, self.string_started)))
        self.current_string = ''
        del self.string_started
        self.in_string = None

    def _Tleft(self, scanner, token):
        if self.in_comment:
            self._skip_token(scanner, token)
        elif self.in_string:
            self._skip_token(scanner, token)
        else:
            new = ParsingNode(self.current)
            self.current.append(new)
            self.current = new

    def _Tright(self, scanner, token):
        if self.in_comment:
            self._skip_token(scanner, token)
        elif self.in_string:
            self._skip_token(scanner, token)
        else:
            self.current = self.current.parent

    def _Ttranslate_next_string(self, scanner, token):
        if self.in_comment:
            self._skip_token(scanner, token)
        elif self.in_string:
            self._skip_token(scanner, token)
        else:
            self.current.append(Token('_', None))

    def _Topen_multiline_string(self, scanner, token):
        if self.in_comment:
            self._skip_token(scanner, token)
        elif self.in_string:
            self._skip_token(scanner, token)
        else:
            self.string_started = self.current_line
            self.in_string = '[['

    def _Tclose_multiline_string(self, scanner, token):
        if self.in_comment:
            self._skip_token(scanner, token)
        elif self.in_string and self.in_string != '[[':
            self._skip_token(scanner, token)
        else:
            assert(self.in_string == '[[')
            self._string_done()

    def _Ttoggle_string(self, scanner, token):
        if self.in_comment:
            self._skip_token(scanner, token)
        elif not self.in_string:
            self.in_string = token
            self.string_started = self.current_line
        elif self.in_string != token:
            self._skip_token(scanner, token)
        else:
            self._string_done()

    def _Tconcat(self, scanner, token):
        if self.in_comment:
            self._skip_token(scanner, token)
        elif self.in_string:
            self._skip_token(scanner, token)
        else:
            self.current.append(Token('..', None))

    def _Tidentifier(self, scanner, token):
        if self.in_comment:
            self._skip_token(scanner, token)
        elif self.in_string:
            self._skip_token(scanner, token)
        else:
            self.current.append(Token(token, None))

    def _Tnewline(self, scanner, token):
        self.current_line += 1
        self._skip_token(scanner, token)
        self.in_comment = False
        if self.translator_comment:
            self.current.append(
                Token('TRANSLATOR_COMMENT', (self.translator_comment.strip(),)))
            self.translator_comment = ''

    def _Twhitespace(self, scanner, token):
        self._skip_token(scanner, token)

    def _Tcomment(self, scanner, token):
        if self.in_comment:
            self._skip_token(scanner, token)
        elif self.in_string:
            self._skip_token(scanner, token)
        else:
            if token.startswith('-- TRANSLATORS'):
                self.translator_comment = 'TRANSLATORS'
            self.in_comment = True


def emit_comments_and_line_numbers(occurences, lines):
    comments = sorted(
        set(f['translator_comment'] for f in occurences if 'translator_comment' in f))
    for comment in comments:
        lines.append('#. %s' % (comment))
    for occurence in occurences:
        lines.append('#: %s:%i' % (os.path.normpath(occurence['filename']),
                             occurence['line']))

def emit_gettext(occurences, lines):
    emit_comments_and_line_numbers(occurences, lines)
    occurence = occurences[0]
    _format_msgid('msgid', occurence['msgid'], lines)
    lines.extend(['msgstr ""', ''])

def emit_ngettext(occurences, lines):
    emit_comments_and_line_numbers(occurences, lines)
    occurence = occurences[0]
    _format_msgid('msgid', occurence['msgid'], lines)
    _format_msgid('msgid_plural', occurence['msgid_plural'], lines)
    lines.append('msgstr[0] ""')
    lines.extend(['msgstr[1] ""', ''])

def emit_pgettext(occurences, lines):
    for context, ctx_occurences in itertools.groupby(occurences, key=lambda a: a['msgctxt']):
        ctx_occurences = list(ctx_occurences)
        emit_comments_and_line_numbers(ctx_occurences, lines)
        occurence = ctx_occurences[0]
        lines.append('msgctxt "%s"' % _escape_pot_string(occurence['msgctxt']))
        _format_msgid('msgid', occurence['msgid'], lines)
        lines.extend(['msgstr ""', ''])

EMIT_FUNCTIONS = {
    'gettext': emit_gettext,
    'ngettext': emit_ngettext,
    'pgettext': emit_pgettext,
}

class Lua_GetText(object):

    def __init__(self):
        self.translatable_items = defaultdict(list)

    def parse(self, contents, filename):
        items = LuaParser().parse(contents)
        for item in items:
            item['filename'] = filename
            self.translatable_items[item['msgid']].append(item)

    @property
    def found_something_to_translate(self):
        return len(self.translatable_items.keys()) > 0

    def merge(self, other_translatable_items):
        for key in other_translatable_items:
            our_translatable_items = self.translatable_items[key]
            for finding in other_translatable_items[key]:
                if isinstance(finding, dict):
                    our_translatable_items.append(finding)
                else:
                    our_translatable_items.append({
                        'type': 'gettext',
                        'msgid': key,
                        'filename': finding[0],
                        'line': finding[1]
                    })

    def __str__(self):
        lines = []

        sort_func = lambda i: [i.get(v, None) for v in (
            'filename', 'line', 'msgid', 'msgid_plural', 'translator_comment')]

        # Now output translatable_items sorted by filename, line number, type. But
        # each msg_id must only be outputted exactly once.
        all_translatable_items = []
        for msgid in self.translatable_items:
            all_translatable_items.extend(self.translatable_items[msgid])
        all_translatable_items.sort(key=sort_func)

        considered_msgids = set()
        for translatable_item in all_translatable_items:
            if translatable_item['msgid'] in considered_msgids:
                continue
            considered_msgids.add(translatable_item['msgid'])

            occurences = self.translatable_items[translatable_item['msgid']]
            occurences.sort(key=sort_func)
            for type, type_occurences in itertools.groupby(occurences, key=lambda a: a['type']):
                emit_function = EMIT_FUNCTIONS[type]
                emit_function(list(type_occurences), lines)
        return head + "\n".join(lines)


def gettext(text, filename):
    t = Lua_GetText()
    t.parse(text, filename)

    return str(t)


if __name__ == '__main__':
    import sys
    t = Lua_GetText()

    for fn in sys.argv[1:]:
        t.parse(open(fn).read(), fn)

    print t
