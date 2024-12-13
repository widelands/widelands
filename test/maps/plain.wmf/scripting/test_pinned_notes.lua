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

   note.text = "<Richtext injection attempt>"
   note.color = {50, 200, 250}
   sleep(5000)

   assert_equal("<Richtext injection attempt>", note.text)
   assert_equal(50, note.color[1])
   assert_equal(1, #f.bobs)

   sleep(1000)
   note:remove()
   sleep(1000)
   assert_equal(0, #f.bobs)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
