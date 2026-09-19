run(function()
   sleep(8000)

   local f = map:get_field(25, 25)
   assert_equal(0, #f.bobs)

   local note = p1:place_pinned_note(f, "Hello World", 255, 0, 123)
   sleep(1000)
   assert_equal(1, #f.bobs)
   assert_equal(note, f.bobs[1])
   assert_equal("Hello World", note.text)
   assert_equal(123, note.color[3])

   -- Show the census overlay so the note's text actually gets rendered as
   -- richtext on the map. Regression test for a crash when the note text
   -- contains a character with special meaning in richtext, e.g. '<'.
   wl.ui.MapView().census = true

   note.text = "<Richtext injection attempt>"
   note.color = {50, 200, 250}
   sleep(5000)

   assert_equal("<Richtext injection attempt>", note.text)
   assert_equal(50, note.color[1])
   assert_equal(1, #f.bobs)

   wl.ui.MapView().census = false

   sleep(1000)
   note:remove()
   sleep(1000)
   assert_equal(0, #f.bobs)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
