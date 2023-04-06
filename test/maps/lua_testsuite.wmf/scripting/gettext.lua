test_descr = lunit.TestCase("Gettext Test")

--  Since we run the test suite in English, this will only test
--  whether gettext functions crash or not

-- Code takes a different path when textdomain was pushed, so we have 2 variants of each test.

function gettext_calls()
   assert_equal("foo", _("foo"))
   assert_equal(_("bar"), _("bar"))
end

function test_descr:test_gettext()
   gettext_calls()
end

function test_descr:test_gettext_push_textdomain()
   push_textdomain("tribes")
   gettext_calls()
   pop_textdomain()
end

local function ngettext_calls()
   assert_equal("foo", ngettext("foo", "bar", 1))
   assert_equal("bar", ngettext("foo", "bar", 1000))
   assert_equal("1 foo", ngettext("%1% foo", "bar %1%", 1):bformat(1))
   assert_equal("bar 2", ngettext("%1% foo", "bar %1%", 2):bformat(2))
   -- ngettext is undefined for floating point or negative numbers.
   assert_error("Call to ngettext with floating point", function()
      ngettext("foo", "bar", 1.5)
   end)
   assert_error("Call to ngettext with negative number", function()
      ngettext("foo", "bar", -1)
   end)
end

function test_descr:test_ngettext()
   ngettext_calls()
end

function test_descr:test_ngettext_push_textdomain()
   push_textdomain("tribes")
   ngettext_calls()
   pop_textdomain()
end


function test_descr:test_pgettext()
   assert_equal("bar", pgettext("foo", "bar"))
end

function test_descr:test_pgettext_push_textdomain()
   push_textdomain("tribes")
   assert_equal("bar", pgettext("foo", "bar"))
   pop_textdomain()
end

local function npgettext_calls()
   assert_equal("foo", npgettext("widelands", "foo", "bar", 1))
   assert_equal("bar", npgettext("widelands", "foo", "bar", 1000))
   assert_equal("1 foo", npgettext("widelands", "%1% foo", "bar %1%", 1):bformat(1))
   assert_equal("bar 2", npgettext("widelands", "%1% foo", "bar %1%", 2):bformat(2))
   -- npgettext is undefined for floating point or negative numbers.
   assert_error("Call to npgettext with floating point", function()
      npgettext("widelands", "foo", "bar", 1.5)
   end)
   assert_error("Call to npgettext with negative number", function()
      npgettext("widelands", "foo", "bar", -1)
   end)
end

function test_descr:test_npgettext()
   npgettext_calls()
end

-- Code takes a different path when textdomain was pushed
function test_descr:test_npgettext_push_textdomain()
   push_textdomain("tribes")
   npgettext_calls()
   pop_textdomain()
end
