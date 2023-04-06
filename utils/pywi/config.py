"""Read and write Widelands-style conf files."""

# TODO(sirver): This should be used for the Website as well.

import collections
import re

SECTION = 1
ENTRY = 2

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

    def __contains__(self, key):
        for line in self.file.lines[self.line:]:
            if line.type == SECTION:
                break
            if line.type == ENTRY and line.key == key:
                return True
        return False

    def iterentries(self):
        for line in self.file.lines[self.line:]:
            if line.type == SECTION:
                break
            if line.type == ENTRY:
                yield line.key, line.value

    def set(self, key, value):
        lastnonempty = -1
        for ofs, line in enumerate(self.file.lines[self.line:]):
            if line.type == ENTRY:
                if line.key == key:
                    self.file.lines[self.line + ofs] = self.file._parse_line(
                        '%s = %s' % (key, value)
                    )
                    ofs += 1
                    while ofs < len(self.file.lines) - self.line:
                        line = self.file.lines[self.line + ofs]
                        if line.type == SECTION:
                            break
                        if line.type == ENTRY and line.key == key:
                            self.file.remove_line(self.line + ofs)
                        else:
                            ofs += 1
                    return

            if line.type == SECTION:
                break
            if line.text:
                lastnonempty = ofs
        self.file.insert_line(self.line + lastnonempty +
                              1, '%s = %s' % (key, value))


class File(object):
    """Represent a Widelands-style conf file.

    This class stores the original source of the file, thus preserving
    comments and formatting where possible.
    """

    def __init__(self):
        self.lines = []
        self._sections = {}

    def write(self, filp):
        for line in self.lines:
            print >>filp, line.text

    def insert_line(self, where, line):
        """Pre-parse and insert the given line."""
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

    def remove_line(self, where):
        if where < 0:
            where = len(self.lines) + where
        del self.lines[where]
        new_sections = {}
        for key, section in self._sections.iteritems():
            if where == key - 1:
                section.line = None
            elif where < key - 1:
                section.line = key - 1
                new_sections[key - 1] = section
        self._sections = new_sections

    def _parse_line(self, line):
        line = line.strip()
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
                    self._sections[idx + 1] = Section(self, idx + 1)
                return self._sections[idx + 1]
        raise Exception('section "%s" not found' % (name))

    def make_section(self, name):
        for idx, line in enumerate(self.lines):
            if line.type == SECTION and line.value == name:
                idx += 1
                while idx < len(self.lines) and self.lines[idx].type != SECTION:
                    self.remove_line(idx)
                self.insert_line(idx, '')
                if idx not in self._sections:
                    self._sections[idx] = Section(self, idx)
                return self._sections[idx]
        if self.lines and self.lines[-1].text:
            self.insert_line(len(self.lines), '')
        self.insert_line(len(self.lines), '[%s]' % (name))
        self._sections[len(self.lines)] = Section(self, len(self.lines))
        return self._sections[len(self.lines)]

    def remove_section(self, name):
        for idx, line in enumerate(self.lines):
            if line.type == SECTION and line.value == name:
                self.remove_line(idx)
                while idx < len(self.lines) and self.lines[idx].type != SECTION:
                    self.remove_line(idx)
                return
        raise KeyError("no section '%s' found" % (name))

    def itersections(self):
        """Iterate through (name, section) pairs."""
        for idx, line in enumerate(self.lines):
            if line.type == SECTION:
                if idx + 1 not in self._sections:
                    self._sections[idx + 1] = Section(self, idx + 1)
                yield (line.value, self._sections[idx + 1])


def read(filp):
    """Read the given file object as a Widelands-style conf file."""
    f = File()
    for line in filp:
        f.insert_line(len(f.lines), line)
    return f
