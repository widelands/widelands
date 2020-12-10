run(function()
   sleep(100)

   assert_equal(1, #p1:get_buildings("barbarians_headquarters"))

   local hqs = p1:get_buildings("barbarians_headquarters")
   local hq_field = hqs[1].flag.fields[1]

   local x = hq_field.x
   local y = hq_field.y

   assert_equal("none", p1:buildhelp(x, y))

   x = x + 1
   assert_equal("medium", p1:buildhelp(x, y))

   y = y + 1
   assert_equal("big", p1:buildhelp(x, y))

   y = y - 2
   assert_equal("small", p1:buildhelp(x, y))

   y = y - 1
   assert_equal("flag", p1:buildhelp(x, y))

   x = x + 1
   y = y - 1
   assert_equal("mine", p1:buildhelp(x, y))

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
