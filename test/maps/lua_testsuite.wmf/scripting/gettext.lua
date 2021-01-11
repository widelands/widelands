test_descr = lunit.TestCase("Gettext Test")

--  Since we run the test suite in English, this will only test
--  whether gettext functions crash or not

function test_descr:test_gettext()
   set_textdomain("tribes")
   assert_equal("foo", _"foo")
   assert_equal("foo", _("foo"))
   assert_equal(_"Carrier", _("Carrier"))
end

function test_descr:test_ngettext()
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

function test_descr:test_pgettext()
   assert_equal("bar", pgettext("foo", "bar"))
end
