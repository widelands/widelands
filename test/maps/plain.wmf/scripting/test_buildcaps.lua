run(function()
   sleep(100)

   assert_equal(1, #p1:get_buildings("barbarians_headquarters"))

   local hqs = p1:get_buildings("barbarians_headquarters")
   local hq_field = hqs[1].flag.fields[1]

   local x = hq_field.x
   local y = hq_field.y


   print("Field 1: " .. p1:buildcaps(x, y))
   assert_equal("none", p1:buildcaps(x, y))

   x = x + 1

   print("Field 2: " .. p1:buildcaps(x, y))
   assert_equal("medium", p1:buildcaps(x, y))

   y = y + 1

   print("Field 3: " .. p1:buildcaps(x, y))
   assert_equal("big", p1:buildcaps(x, y))

   y = y - 2
   print("Field 4: " .. p1:buildcaps(x, y))
   assert_equal("small", p1:buildcaps(x, y))

   y = y - 1
   print("Field 5: " .. p1:buildcaps(x, y))
   assert_equal("flag", p1:buildcaps(x, y))

   x = x + 1
   y = y - 1
   print("Field 6: " .. p1:buildcaps(x, y))
   assert_equal("mine", p1:buildcaps(x, y))

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
