
function mission_thread()
   sleep(100) -- This is needed for yet unknown reasons
   
   -- Initial messages
   local sea = wl.map.Field(47,25)
   local pts = scroll_smoothly_to(sea)

   send_msg(diary_page_1)
   sleep(200)

   -- Show the sea
   p1:reveal_fields(sea:region(6))
   sleep(200)
   send_msg(diary_page_2)
   -- Hide the sea after 5 seconds
   run(function() sleep(5000) p1:hide_fields(sea:region(6)) end)

   -- Back home
   timed_scroll(array_reverse(pts))
   send_msg(diary_page_3)


   sleep(400)
   
   send_msg(saledus_1)
   p1:allow_buildings{"barracks"}
   local o = add_obj(obj_build_barracks)
   while #p1:get_buildings("barracks") < 1 do sleep(3249) end
   o.done = true

   -- Barracks are completed now
   send_msg(saledus_2)
   p1:allow_buildings{"lumberjacks_house"}
   o = add_obj(obj_build_lumberjack)
   send_msg(amalea_1)
   while #p1:get_buildings("lumberjacks_house") < 1 do sleep(3249) end
   o.done = true

   -- Lumberjack is now build
   send_msg(amalea_2)
   p1:allow_buildings{"sawmill"}
   o = add_obj(obj_build_sawmill_and_lumberjacks)
   while not check_for_buildings(p1, { lumberjacks_house = 3, sawmill = 1})
      do sleep(2343) end
   o.done = true

   -- Now the lady demands a forester after having us cut down the whole forest.
   send_msg(amalea_3)
   o = add_obj(obj_build_forester)
   p1:allow_buildings{"foresters_house"}
   while not check_for_buildings(p1, { foresters_house = 1 }) do sleep(2434) end
   o.done = true


   -- Now a quarry
   send_msg(saledus_3)
   o = add_obj(obj_build_quarry)
   p1:allow_buildings{"quarry"}
   while not check_for_buildings(p1, { quarry = 1 }) do sleep(2434) end
   o.done = true

   -- All buildings done. Got home
   send_msg(saledus_4)

   sleep(25000) -- Sleep a while

   send_msg(diary_page_4)
   p1:reveal_scenario("empiretut01")
end

-- Show a funny message when the player has build 10 barracks
function easter_egg()
   while not check_for_buildings(p1, {barracks = 10}) do sleep(4253) end

   send_msg(safe_peninsula)
end


run(mission_thread)
run(easter_egg)

