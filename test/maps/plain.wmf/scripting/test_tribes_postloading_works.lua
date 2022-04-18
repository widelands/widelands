include "test/scripting/stable_save.lua"

run(function()
   sleep(5000)

   assert_equal(1, #p1:get_buildings("barbarians_headquarters"))

   local hqs = p1:get_buildings("barbarians_headquarters")
   local hq = hqs[1]
   local fhq = hq.flag
   hq:set_workers("barbarians_geologist", 2)

   local field = map:get_field(25,22)
   local flag = p1:place_flag(field)
   local road = p1:place_road("normal", flag, "bl", "bl", "bl", "bl")
   sleep(1000)

   stable_save(game, "before_sending_geologist", 2 * 1000)

   local mv = wl.ui.MapView()
   mv:click(field)
   sleep(1000)

   mv.windows.field_action.buttons.geologist:click()
   game.desired_speed = 5000
   sleep(1000)

   for i=1,36 do
      sleep(10000)
      for i,f in pairs(field:region(6)) do
         if f.immovable and (
            f.immovable.descr.name == "barbarians_resi_coal_1" or
            f.immovable.descr.name == "barbarians_resi_iron_1" or
            f.immovable.descr.name == "barbarians_resi_gold_1" or
            f.immovable.descr.name == "barbarians_resi_stones_1" or
            f.immovable.descr.name == "barbarians_resi_coal_2" or
            f.immovable.descr.name == "barbarians_resi_iron_2" or
            f.immovable.descr.name == "barbarians_resi_gold_2" or
            f.immovable.descr.name == "barbarians_resi_stones_2"
         ) then
            print("# All Tests passed.")
            wl.ui.MapView():close()
            return
         end
      end
   end

   print("Postloading bug!")
   assert(false)
end)
