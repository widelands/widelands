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

        # Make sure we didn't find anything more then expected
        nfindings = sum(len(self.p.findings[i]) for i in self.p.findings.keys())
        self.assertEqual(len(self.items), nfindings)

        for item in self.items:
            msg_id, count, line = item[:3]
            msgid_plural = None
            translator_comment = ""
            if len(item) == 4:
                msgid_plural = item[3]
            if len(item) == 5:
                translator_comment = item[4]
            self.assertEqual(self.p.findings[msg_id][count].filename, self.filename)
            self.assertEqual(self.p.findings[msg_id][count].lineno, line)
            self.assertEqual(self.p.findings[msg_id][count].msgid_plural, msgid_plural)
            self.assertEqual(self.p.findings[msg_id][count].translator_comment, translator_comment)


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
        ("a lumberjack'ssecond linethird line.", 0, 4),
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

class TestNgettextStuff(_TestLua_GetText_SingleFile):
    items = [
        ("car", 0, 3, "cars"),
    ]
    code = """
function a()
    local p = ngettext("car", "cars", item)
    end
"""

class TestTranslatorComment(_TestLua_GetText_SingleFile):
    items = [
        ("blub", 0, 3, None, """TRANSLATORS: This is "blub" 'test'"""),
    ]
    code = """
-- TRANSLATORS: This is "blub" 'test'
function a() return _ "blub" end
"""

class TestTranslatorCommentMultiline(_TestLua_GetText_SingleFile):
    items = [
        ("blub", 0, 4, None, """TRANSLATORS: This is "blub" 'test'\nTRANSLATORS: And this should be clear"""),
    ]
    code = """
-- TRANSLATORS: This is "blub" 'test'
-- TRANSLATORS: And this should be clear
function a() return _ "blub" end
"""

class TestRealWorldExample(_TestLua_GetText_SingleFile):
    items = [
     ("A young man approaches", 0, 2),
("May Satul warm you, Jundlina. My name is Ostur and I construct ships. I have\ninvented a new kind of ship: smaller than those we are used to, but much\nsturdier. If we build them correctly, I am confident that we can go with them\na much longer distance and maybe escape from Lutas' influence.\n", 0, 4)
]
    code ="""{
   title = _ "A young man approaches",
   body = ostur(_
[[May Satul warm you, Jundlina. My name is Ostur and I construct ships. I have
invented a new kind of ship: smaller than those we are used to, but much
sturdier. If we build them correctly, I am confident that we can go with them
a much longer distance and maybe escape from Lutas' influence.
]])
},
"""

class TestRealWorldExample1(_TestLua_GetText_SingleFile):
    items = [
        ("%s has been King of the Hill since %s!", 0, 1)
    ]
    code = """had_control_for = rt(p(_[[%s has been King of the Hill since %s!]]))"""

class TestRealWorldExample2(_TestLua_GetText_SingleFile):
    items = [
        ("An old man says...", 0, 5),
        (' "Hail, chieftain. I am Khantrukh and have seen many winters pass. Please allow me to aid you with my counsel through these darkened days." ', 0, 7)
    ]
    code = """
-- Khantruth's texts
-- Khantruth"s texts
khantrukh_1="<rt><p font-size=24 font-face=DejaVuSerif font-weight=bold font-color=8080FF>" ..
_"An old man says..." ..
"</p></rt><rt image=map:khantrukh.png><p line-spacing=3 font-size=12>" ..
_[[ "Hail, chieftain. I am Khantrukh and have seen many winters pass. Please allow me to aid you with my counsel through these darkened days." ]] ..
"</p></rt>"
"""



if __name__ == '__main__':
   unittest.main()
   k = SomeTestClass()
   unittest.TextTestRunner().run(k)
