-- =======================================================================
--                              Mission Threads                             
-- =======================================================================
function building_materials()
   sleep(200)
   send_msg(diary_page_5)

   local map = wl.Game().map
   local forests = map:get_field(17,10)
   p1:reveal_fields(forests:region(6))
   -- Hide again in 5 seconds
   run(function() sleep(5000) p1:hide_fields(forests:region(6)) end)
   
   -- Show the trees
   scroll_smoothly_to(forests)

   send_msg(diary_page_5_1)
   local o_woodeconomy = add_obj(obj_build_woodeconomy)
   -- Check for completing of the woodeconomy
   run(function()
      while not check_for_buildings(p1, {
         lumberjacks_house = 3,
         foresters_house = 1,
         sawmill = 1,
      }) do sleep(2342) end

      send_msg(diary_page_6)
      o_woodeconomy.done = true
   end)


   -- Show the stones
   scroll_smoothly_to(map:get_field(14,25))
   send_msg(diary_page_5_2)
   local o_quarry = add_obj(obj_build_quarry)
   -- Check for completeness of the quarry
   run(function()
      while not check_for_buildings(p1, {quarry=1}) do sleep(3423) end
      o_quarry.done = true

      send_msg(diary_page_7)
   end)
end

function food_thread()
   -- Wait till one mine goes up
   while true do
      local rv = p1:get_buildings{
         "coalmine",
         "deep_coalmine",
         "oremine",
         "deep_oremine",
         "marblemine",
      }
      local s = 0
      for idx,a in pairs(rv) do s = s + #a end
      if s > 0 then break end
      sleep(2344)
   end
   
   send_msg(amalea_1)
   p1:allow_buildings{
      "well",
      "farm",
      "brewery",
      "vineyard",
      "winery",
      "mill",
      "bakery",
      "hunters_house",
      "piggery",
      "tavern",
      "inn",
      "fishers_house",
   }
   local o = add_obj(obj_build_Food_infrastructure)

   -- Run easter egg: Amalea says something when Tavern is done
   run(function() 
      while #p1:get_buildings("tavern") < 1 do sleep(2349) end
      send_msg(amalea_2)
   end)

   while true do
      local rv = p1:get_buildings{
         "brewery",
         "fishers_house",
         "bakery",
         "hunters_house",
         "piggery",
         "tavern",
         "inn",
      }
      if #rv.brewery > 0 and
         (#rv.tavern + #rv.inn) > 0 and
         (#rv.fishers_house + #rv.hunters_house +
          #rv.piggery + #rv.bakery) > 0 then break end
      sleep(4857)
   end
   o.done = true
end

function mining_infrastructure()
   -- Wait till we see the mountains
   local mountains = wl.Game().map:get_field(38,33)
   while not p1:seen_field(mountains) do sleep(3458) end

   -- Reveal the other mountains
   local coal_mountain = wl.Game().map:get_field(49,22)
   local iron_mountain = wl.Game().map:get_field(38,37)
   p1:reveal_fields(coal_mountain:region(6))
   p1:reveal_fields(iron_mountain:region(6))
   run(function() sleep(5000) 
      p1:hide_fields(coal_mountain:region(6))
      p1:hide_fields(iron_mountain:region(6))
   end)

   local move_point = wl.Game().map:get_field(49,22)
   scroll_smoothly_to(move_point)

   send_msg(saledus_3)
   p1:allow_buildings{
      "coalmine",
      "deep_coalmine",
      "oremine",
      "deep_oremine",
      "stonemasons_house",
      "toolsmithy",
      "armorsmithy",
      "weaponsmithy",
      "smelting_works",
      "burners_house",
   }

   local o = add_obj(obj_build_mining_infrastructure)
   -- Wait for the infrastructure to come up 
   while true do
      local rv = p1:get_buildings{
         "coalmine",
         "deep_coalmine",
         "oremine",
         "deep_oremine",
         "stonemasons_house",
         "toolsmithy",
         "armorsmithy",
         "weaponsmithy",
         "smelting_works",
         "burners_house",
      }
      if #rv.stonemasons_house > 0 and
         #rv.toolsmithy > 0 and
         #rv.armorsmithy > 0 and
         #rv.smelting_works > 0 and
         (#rv.oremine + #rv.deep_oremine > 0) and
         (#rv.coalmine + #rv.deep_coalmine + #rv.burners_house > 0) and
         #rv.weaponsmithy > 0 then break end
      sleep(4948)
   end
   o.done = true
   mining_infrastructure_done = true
end

function expand_and_build_marblemine()
   sleep(40000)

   local shipparts = wl.Game().map:get_field(15,46)
   p1:reveal_fields(shipparts:region(5))
   run(function() sleep(10000) p1:hide_fields(shipparts:region(5)) end)

   -- Move to the shipparts
   local pts = scroll_smoothly_to(shipparts)

   send_msg(saledus_1)
   local o = add_obj(obj_build_military_buildings)
   p1:allow_buildings{"barracks", "sentry"}

   -- Go back to where we were
   timed_scroll(array_reverse(pts))

   -- sleep while not owning 26, 21
   while wl.Game().map:get_field(26,21).owner ~= p1 do sleep(3243) end
   o.done = true
   
   -- Marble Mountains
   local marblemountains = wl.Game().map:get_field(35,19)
   p1:reveal_fields(marblemountains:region(5))
   run(function() sleep(10000) p1:hide_fields(marblemountains:region(5)) end)

   pts = scroll_smoothly_to(marblemountains)

   send_msg(saledus_2)
   p1:allow_buildings{"marblemine", "deep_marblemine"}
   o = add_obj(obj_build_marblemine)
   run(function() while not check_for_buildings(p1, {marblemine = 1})
      do sleep(2133) end  o.done = true end)

   -- Go back to where we were
   timed_scroll(array_reverse(pts))
end

function barbarians_thread()
   -- Wait for various buildings to go up
   while true do
      local rv = p1:get_buildings{
         "marblemine", "deep_marblemine", "oremine", "deep_oremine",
         "coalmine", "deep_coalmine", "burners_house"
      }
      local mm = #rv.marblemine + #rv.deep_marblemine
      local ore = #rv.oremine + #rv.deep_oremine
      local coal = #rv.coalmine + #rv.deep_coalmine + #rv.burners_house

      if mm > 0 and ore > 0 and coal > 0 then break end
      sleep(6674)
   end

   -- show barbarians
   local barbarians = array_combine(
      wl.Game().map:get_field(60, 61):region(6),
      wl.Game().map:get_field(60, 48):region(6)
   )
   p1:reveal_fields(barbarians)
   run(function() sleep(5000) p1:hide_fields(barbarians) end)
   scroll_smoothly_to(wl.Game().map:get_field(59, 55))

   send_msg(diary_page_8)
   local o = add_obj(obj_build_bigger_military_buildings)
   p1:allow_buildings{"outpost", "barrier", "tower"}
   p2:allow_buildings{"quarry"}

   -- Wait for one of the buildings to go up
   while true do
      local rv = p1:get_buildings{"outpost", "barrier", "tower"}
      if (#rv.tower + #rv.outpost + #rv.barrier) > 0 then
         break
      end
      sleep(2342)
   end
   o.done = true

   -- Wait till the mining infrastructure is also up
   while not mining_infrastructure_done do
      sleep(2343)
   end
   send_msg(diary_page_9)
   p1:allow_buildings{"fortress"}

   o = add_obj(obj_remove_the_barbarians)
   -- Wait for the fortress to come up
   while not check_for_buildings(p1, {fortress=1},
      wl.Game().map:get_field(60,65):region(6))
   do sleep(2435) end

   o.done = true

   -- Show victory message
   send_msg(diary_page_10)
   send_msg(seven_days_later)
   send_msg(diary_page_11)

   p1:reveal_campaign("campsect2")
   p1:reveal_scenario("empiretut02")
end

run(building_materials)
run(food_thread)
run(mining_infrastructure)
run(expand_and_build_marblemine)
run(barbarians_thread)

