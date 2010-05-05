#!/usr/bin/env python
# encoding: utf-8

import unittest

import os, sys
sys.path.append(os.path.dirname(__file__) + os.path.sep + '..')

from lua_xgettext import gettext, Lua_GetText, head

class TestXGettext(unittest.TestCase):
    def runTest(self):
        self.assertTrue(True)

    def test_empty_input(self):
        self.assertEqual(head,gettext("", "test.lua"))

class _TestLua_GetText_SingleFile(unittest.TestCase):
    filename = "test.lua"

    def setUp(self):
        self.p = Lua_GetText()

    def run_test(self):
        self.p.parse(self.code, self.filename)

        print self.p.findings

        # Make sure we didn't find anything more then expected
        nfindings = sum(len(self.p.findings[i]) for i in self.p.findings.keys())
        self.assertEqual(len(self.items), nfindings)

        for entry, count, line in self.items:
            self.assertTrue(entry in self.p.findings, "'%s' not found!" % entry)
            self.assertEqual(self.p.findings[entry][count],
                             (self.filename, line)
            )


##################
# Simple Strings #
##################
class TestSimpleString_DoubleQuotes(_TestLua_GetText_SingleFile):
    items = [
        ("Hello World", 0, 2),
    ]
    code ='''
print _ "Hello World"
'''
class TestSimpleStringWithParens_DoubleQuotes(_TestLua_GetText_SingleFile):
    items = [
        ("Hello World", 0, 2),
    ]
    code ='''
print _("Hello World"  )
'''
class TestSimpleString_SingleQuotes(_TestLua_GetText_SingleFile):
    items = [
        ("Hello World", 0, 2),
    ]
    code ='''
print _ 'Hello World'
'''
class TestSimpleStringWithParens_SingleQuotes(_TestLua_GetText_SingleFile):
    items = [
        ("Hello World", 0, 2),
    ]
    code ='''
print _('Hello World'  )
'''
class TestSimpleStringWithParens_Inline(_TestLua_GetText_SingleFile):
    items = [
        ("This is ", 0, 3),
        ("illegal!", 0, 3),
    ]
    code ='''
    function a()
        if(nil) then return _"This is " .. _( "illegal!" ) end
    end
'''

class TestSimpleStringWithBackslashes_Inline(_TestLua_GetText_SingleFile):
    items = [
        (r"""This " is """, 0, 3),
        (r"fin'e!", 0, 3),
    ]
    code =r'''
    function a()
        if(nil) then return _"This \" is " .. _( "fin'e!" ) end
    end
'''
class TestSimpleStringWithBackslashes_SQuote(_TestLua_GetText_SingleFile):
    items = [
        (r"This ' is ", 0, 3),
        (r'fin"e!', 0, 3),
    ]
    code =r'''
    function a()
        if(nil) then return _'This \' is ' .. _( 'fin"e!' ) end
    end
'''

########################
# Concatenated strings #
########################
class TestConcatStrings_Inline(_TestLua_GetText_SingleFile):
  items = [
        ("This is ", 0, 3),
    ]
  code = '''
    function a()
        if(nil) then return _"This is " .. "fine!"  end
    end
    '''

class TestConcatStrings_WithParen(_TestLua_GetText_SingleFile):
  items = [
        ("This is fine!", 0, 3),
    ]
  code = '''
    function a()
        if(nil) then return _ ("This is " .. "fine!")  end
    end
    '''
class TestConcatStrings_WithParenMultiLine(_TestLua_GetText_SingleFile):
  items = [
        ("This is fine!", 0, 4),
    ]
  code = '''
    function a()

        if(nil) then return _ ("This " ..
        "is "
         .. "fine!")  end
    end
    '''
class TestConcatStrings_MixQuotes(_TestLua_GetText_SingleFile):
  items = [
        ("This is fine!", 0, 3),
    ]
  code = '''
    function a()
        if(nil) then return _ ("This " ..  'is ' .. "fine!")  end
    end
    '''
class TestConcatStrings_MixQuotes1(_TestLua_GetText_SingleFile):
  items = [
        ("This is fine!", 0, 4),
    ]
  code = '''
    a = 100
    function a()
        if(nil) then return _ ('This ' ..  "is " .. 'fine!')  end
    end
    '''
class TestConcatStrings_Difficult(_TestLua_GetText_SingleFile):
    items = [
        ("a lumberjack'ssecond linethird line.", 0, 3),
    ]

    code = '''
msg = "a" ..
_(
   "a lumberjack's" ..
   "second line" ..
   "third line."
   ) ..
"b",
'''

#####################
# Multiline Strings #
#####################
class TestMultilineStrings_Simple(_TestLua_GetText_SingleFile):
  items = [
        ("This\nis that then", 0, 3),
    ]
  code = '''
    function a()
        if(nil) then return _ [[This
is that then]]
        end
    end
    '''

class TestMultilineStrings_SimpleNoTrans(_TestLua_GetText_SingleFile):
  items = [
        ("This\nis that then", 0, 3),
    ]
  code = '''
    function a()
        if(nil) then return _ [[This
is that then]] .. [[More]] .. "And more"
        end
    end
    '''

class TestMultilineStrings_SimpleNoTrans1(_TestLua_GetText_SingleFile):
  items = [
        ("This\nis that then", 0, 3),
        ("And more", 0, 4),
    ]
  code = '''
    function a()
        if(nil) then return _ [[This
is that then]] .. [[More]] .. _ "And more"
        end
    end
    '''
class TestMultilineStrings_AllTrans(_TestLua_GetText_SingleFile):
  items = [
        ("This\nis that then", 0, 3),
        ("More", 0, 4),
        ("And more", 0, 4),
    ]
  code = '''
    function a()
        if(nil) then return _ [[This
is that then]] .. _[[More]] .. _ "And more"
        end
    end
    '''

class TestMultilineStrings_InParen(_TestLua_GetText_SingleFile):
  items = [
        ("This\nis that thenMoreAnd more", 0, 3),
    ]
  code = '''
    function a()
        if(nil) then return _ ([[This
is that then]] .. [[More]] ..  "And more"
)
        end
    end
    '''

class TestMultilineStrings_ContainingSingleLine(_TestLua_GetText_SingleFile):
  items = [
        ('"Lorem ipsum dolor sit amet, consetetur sadipscing\nelitr, sed diam '
         'nonumy eirmod tempor"', 0, 4),
    ]
  code = '''
    function a()
        if(nil) then return _
[["Lorem ipsum dolor sit amet, consetetur sadipscing
elitr, sed diam nonumy eirmod tempor"]]
        end
    end
    '''

class TestVerySimpleMultilineString(_TestLua_GetText_SingleFile):
    items = [
        (' "There is an old saying:<br> blah ', 0, 1),
    ]
    code = """_ [[ "There is an old saying:<br> blah ]]"""


if __name__ == '__main__':
   unittest.main()
   # k = SomeTestClass()
   # unittest.TextTestRunner().run(k)

