include "scripting/messages.lua"

function mission_thread()
   sleep(1000)

   -- Initial messages
   local sea = wl.Game().map:get_field(50,25)
   scroll_smoothly_to(sea,0)

   campaign_message_box(diary_page_1)
   sleep(200)

   -- Show the sea
   p1:reveal_fields(sea:region(6))
   local ship = p1:place_ship(sea)
   sleep(1000)
   campaign_message_box(diary_page_2)
   -- Hide the sea after 5 seconds
   run(function() sleep(5000) p1:hide_fields(sea:region(6)) end)

   -- Back home
   include "map:scripting/starting_conditions.lua"
   scroll_smoothly_to(wl.Game().map.player_slots[1].starting_field)
   campaign_message_box(diary_page_3)
   ship:remove()

   sleep(400)

   campaign_message_box(saledus_1)
   p1:allow_buildings{"empire_blockhouse"}
   local o = add_campaign_objective(obj_build_blockhouse)
   while #p1:get_buildings("empire_blockhouse") < 1 do sleep(3249) end
   o.done = true

   -- Blockhouse is completed now
   campaign_message_box(saledus_2)
   p1:allow_buildings{"empire_lumberjacks_house"}
   o = add_campaign_objective(obj_build_lumberjack)
   campaign_message_box(amalea_1)
   while #p1:get_buildings("empire_lumberjacks_house") < 1 do sleep(3249) end
   o.done = true

   -- Lumberjack is now build
   campaign_message_box(amalea_2)
   p1:allow_buildings{"empire_sawmill"}
   o = add_campaign_objective(obj_build_sawmill_and_lumberjacks)
   while not check_for_buildings(p1, { empire_lumberjacks_house = 3, empire_sawmill = 1})
      do sleep(2343) end
   o.done = true

   -- Now the lady demands a forester after having us cut down the whole forest.
   campaign_message_box(amalea_3)
   o = add_campaign_objective(obj_build_forester)
   p1:allow_buildings{"empire_foresters_house"}
   while not check_for_buildings(p1, { empire_foresters_house = 1 }) do sleep(2434) end
   o.done = true


   -- Now a quarry
   campaign_message_box(saledus_3)
   o = add_campaign_objective(obj_build_quarry)
   p1:allow_buildings{"empire_quarry"}
   while not check_for_buildings(p1, { empire_quarry = 1 }) do sleep(2434) end
   o.done = true

   -- All buildings done. Got home
   campaign_message_box(saledus_4)

   sleep(25000) -- Sleep a while

   campaign_message_box(diary_page_4)
   p1:reveal_scenario("empiretut01")
end

-- Show a funny message when the player has build 10 blockhouses
function easter_egg()
   while not check_for_buildings(p1, {empire_blockhouse = 10}) do sleep(4253) end

   campaign_message_box(safe_peninsula)
end


run(mission_thread)
run(easter_egg)