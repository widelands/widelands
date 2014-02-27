#!/usr/bin/env python
# encoding: utf-8

from collections import defaultdict, namedtuple
import re

from confgettext import head

Token = namedtuple('Token', ['type', 'data'])

class ParsingNode(list):
    def __init__(self, parent=None):
        self.parent = parent
        self.type = "NODE"

    def __str__(self):
        return "N%s" % list.__str__(self)


class LuaParser(object):
    def __init__(self):
        self.scanner = re.Scanner([
            (r"\-\-", self._Tcomment),
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
        self.current_string = ""
        self.result = ParsingNode()
        self.in_comment = False
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
                if i > 0 and l[i].type == "..":
                    if l[i-1].type != "STRING" or l[i+1].type != "STRING":
                        continue
                    # Do not concatenate translated and untranslated string.
                    if i-2 >= 0 and l[i-2].type == "_":
                        continue
                    first_string, line = l[i-1].data
                    second_string, unused = l[i+1].data
                    l[i-1:i+2] = [ Token("STRING", (first_string + second_string, line)) ]
                    return _combine_concatenated_strings(l)
            return l

        strings = []
        def _find_translatable_strings(l):
            for i in range(len(l)):
                if l[i].type == "_":
                    if l[i+1].type == "NODE":
                        strings.append(l[i+1][0].data)
                    else:
                        strings.append(l[i+1].data)
                if l[i].type == "ngettext":
                    assert l[i+1].type == "NODE"
                    assert len(l[i+1]) >= 4
                    data = l[i+1][0].data + (l[i+1][2].data[0],)
                    strings.append(data)
            return l

        def _remove_empties(l):
            rv = ParsingNode()
            for a in l:
                if isinstance(a, list) and not a:
                    continue
                rv.append(a)
            return rv

        self.result = _recurse_lists(self.result, _remove_empties)
        self.result = _recurse_lists(self.result, _combine_concatenated_strings)

        _recurse_lists(self.result, _find_translatable_strings)
        return strings

    def _skip_token(self, scanner, token):
        if self.in_string:
            self.current_string += token

    def _string_done(self):
        text = eval('str(""" %s """)' % self.current_string)[1:-1]
        self.current.append(Token("STRING", (text, self.string_started)))
        self.current_string = ""
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
            self.current.append(Token("_", None))

    def _Topen_multiline_string(self, scanner, token):
        if self.in_comment:
            self._skip_token(scanner, token)
        elif self.in_string:
            self._skip_token(scanner, token)
        else:
            self.string_started = self.current_line
            self.in_string = "[["

    def _Tclose_multiline_string(self, scanner, token):
        if self.in_comment:
            self._skip_token(scanner, token)
        elif self.in_string and self.in_string != "[[":
            self._skip_token(scanner, token)
        else:
            assert(self.in_string == "[[")
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
            self.current.append(Token("..", None))

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

    def _Twhitespace(self, scanner, token):
        self._skip_token(scanner, token)

    def _Tcomment(self, scanner, token):
        if self.in_comment:
            self._skip_token(scanner, token)
        elif self.in_string:
            self._skip_token(scanner, token)
        else:
            self.in_comment = True


class Lua_GetText(object):
    def __init__(self):
        self.findings = defaultdict(list)

    def parse(self, contents, filename):
        items = LuaParser().parse(contents)
        for item in items:
            if len(item) == 2:
                msg_id, line = item
                self.findings[msg_id].append(
                    (filename, line)
                )
            elif len(item) == 3:
                msg_id, line, plural_id = item
                self.findings[msg_id].append(
                    (filename, line, plural_id)
                )

    @property
    def found_something_to_translate(self):
        return len(self.findings.keys()) > 0

    def merge(self, other_findings):
        for key in other_findings:
            self.findings[key].extend(other_findings[key])

    def __str__(self):
        s = head

        def _output_string(description, string):
            # there was a bug in this code that would never output single line
            # msg_id's. For consistency, I decided to not fix this bug, but
            # instead keep it around.
            # if not string.count('\n'): <== this was s.count
                # s += 'msgid "%s"\n' % string
            # else:
            output = ""
            string = string.replace('\\', '\\\\').replace('"', '\\"')
            output += '%s ""\n' % description
            lines = string.split('\n')
            output += ''.join('"%s\\n"\n' % l for l in lines[:-1])
            output += '"%s"\n' % lines[-1]
            return output

        for string in sorted(self.findings.keys(),key=self.findings.get):
            occurences = self.findings[string]
            occurences.sort() # Sort by filename and lines

            for occurence in occurences:
                assert(len(occurence) == len(occurences[0]))
                s += "#: %s:%i\n" % (occurence[0], occurence[1])

            if len(occurence) == 2:
                s += _output_string("msgid", string)
                s += 'msgstr ""\n\n'
            if len(occurence) == 3:
                s += _output_string("msgid", string)
                s += _output_string("msgid_plural", occurence[2])
                s += 'msgstr[0] ""\n'
                s += 'msgstr[1] ""\n\n'
        return s


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
