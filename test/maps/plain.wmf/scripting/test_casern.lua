run(function()
   sleep(5000)

   assert_equal(1, #p1:get_buildings("barbarians_headquarters"))

   local hqs = p1:get_buildings("barbarians_headquarters")
   local hq = hqs[1]
   local fhq = hq.flag

   prefilled_buildings(p1, { "barbarians_barracks", 25, 25, })

   local brs = p1:get_buildings("barbarians_barracks")
   local br = brs[1]

   -- Check if empty and accepts the right input
   assert_error("wrong worker", function()
      br:set_inputs{"barbarians_builder", 1}
   end)
   assert_error("illegal input", function()
      br:set_inputs("balloon",1)
   end)
   assert_error("negative counts", function()
      br:set_inputs("barbarians_recruit", -1)
   end)
   rv = br.valid_inputs
   assert_equal(8, rv.ax)
   assert_equal(8, rv.barbarians_recruit)
   assert_equal(nil, rv.meat)

   br:set_inputs{barbarians_recruit=3}
   rv = br:get_inputs("all")
   assert_equal(0, rv.ax)
   assert_equal(3, rv.barbarians_recruit)
   assert_equal(nil, rv.meat)

   -- Sleep long enough to train a soldier. But ax are missing so nothing should happen
   sleep(40000)
   rv = br:get_inputs("all")
   assert_equal(0, rv.ax)
   assert_equal(3, rv.barbarians_recruit)

   -- Same but with ax and no recruits
   br:set_inputs{ax=3, barbarians_recruit=0}
   rv = br:get_inputs("all")
   assert_equal(3, rv.ax)
   assert_equal(0, rv.barbarians_recruit)
   sleep(40000)
   rv = br:get_inputs("all")
   assert_equal(3, rv.ax)
   assert_equal(0, rv.barbarians_recruit)

   -- Connect road to hq, start training
--   local road = p1:place_road(fhq, "r", "r", "r")
   connected_road(p1, fhq, "r,r,r|", true)
   br:set_inputs{barbarians_recruit=3}
   rv = br:get_inputs("all")
   assert_equal(3, rv.ax)
   assert_equal(3, rv.barbarians_recruit)

   rv = hq:get_wares("all")
   assert_equal(0, rv.ax)
   rv = hq:get_workers("all")
   -- No sense checking for recruits, they are for free
   assert_equal(0, rv.barbarians_soldier)

   sleep(150000)

   rv = br:get_inputs("all")
   assert_equal(0, rv.ax)
   assert_equal(8, rv.barbarians_recruit)

   rv = hq:get_wares("all")
   assert_equal(0, rv.ax)
   rv = hq:get_workers("all")
   assert_equal(3, rv.barbarians_soldier)

   -- Ax out of the warehouse
   hq:set_wares{ax=2}
   sleep(100000)

   rv = br:get_inputs("all")
   assert_equal(0, rv.ax)
   assert_equal(8, rv.barbarians_recruit)

   rv = hq:get_wares("all")
   assert_equal(0, rv.ax)
   rv = hq:get_workers("all")
   assert_equal(5, rv.barbarians_soldier)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)