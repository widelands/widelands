"""
Read and write Widelands-style conf files
"""

import collections
import re

SECTION=1
ENTRY=2

Line = collections.namedtuple('Line', ('text', 'type', 'key', 'value'))

_re_section = re.compile('\[(.+?)\]')
_re_entry = re.compile('(\\S+?)\\s*=_?\\s*([^#]*)')

class Section(object):
    def __init__(self, file, line):
        self.file = file
        self.line = line

    def __getitem__(self, key):
        for line in self.file.lines[self.line:]:
            if line.type == SECTION:
                break
            if line.type == ENTRY:
                if line.key == key:
                    return line.value
        raise KeyError('key "%s" not found' % (key))

    def iterentries(self):
        for line in self.file.lines[self.line:]:
            if line.type == SECTION:
                break
            if line.type == ENTRY:
                yield line.key, line.value


class File(object):
    """
    Represent a Widelands-style conf file.

    This class stores the original source of the file,
    thus preserving comments and formatting where possible.
    """
    def __init__(self):
        self.lines = []
        self._sections = {}

    def insert_line(self, where, line):
        """
        Pre-parse and insert the given line
        """
        if where < 0:
            where = len(self.lines) + where
        self.lines.insert(where, self._parse_line(line))
        new_sections = {}
        for key, section in self._sections.iteritems():
            if where >= key:
                new_sections[key] = section
            else:
                section.line = key + 1
                new_sections[key + 1] = section
        self._sections = new_sections

    def _parse_line(self, line):
        line = line.lstrip()
        if not line or line[0] == '#':
            return Line(line, 0, None, None)

        if line[0] == '[':
            m = _re_section.match(line)
            if not m:
                raise Exception('incomplete section definition')
            return Line(line, SECTION, None, m.group(1))

        m = _re_entry.match(line)
        if m:
            return Line(line, ENTRY, m.group(1).strip(), m.group(2).strip())

        raise Exception('failed to parse line')

    def get_section(self, name):
        for idx, line in enumerate(self.lines):
            if line.type == SECTION and line.value == name:
                if idx not in self._sections:
                    self._sections[idx] = Section(self, idx + 1)
                return self._sections[idx]
        raise Exception('section "%s" not found' % (name))


def read(filp):
    """
    Read the given file object as a Widelands-style conf file.
    """
    f = File()
    for line in filp:
        f.insert_line(len(f.lines), line)
    return f

def write(filp, sections):
    """
    Write the given dictionary of sections into the given
    """