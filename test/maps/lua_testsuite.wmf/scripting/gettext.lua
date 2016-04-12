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
   -- TODO(GunChleoc): Just using the floor here is wrong for English,
   -- but an edge case as compared to other languages that need it here.
   -- ngettext doesn't actually support floating point numbers,
   -- so we might want to create our own rules off the CLDR some time.
   assert_equal("foo", ngettext("foo", "bar", 1.5))
   assert_equal("bar", ngettext("foo", "bar", 1000))
   assert_equal("bar", ngettext("foo", "bar", 1000.5))
end

function test_descr:test_pgettext()
   assert_equal("bar", pgettext("foo", "bar"))
end
