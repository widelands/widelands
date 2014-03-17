-- =======================================================================
--                          Various mission threads
-- =======================================================================

-- Mountain and frontier fields
mountain = wl.Game().map:get_field(71,14)
fr1 = wl.Game().map:get_field(81,108)
fr2 = wl.Game().map:get_field(85,1)
fr3 = wl.Game().map:get_field(85,11)

function remember_cattlefarm()
   sleep(100)

   send_msg(cattlefarm_01)

   p1:allow_buildings{"cattlefarm"}
   local o = add_obj(obj_build_cattlefarm)
   while not check_for_buildings(p1, {cattlefarm = 1}) do
      sleep(1234) end
   o.done = true

end

function initial_message_and_small_food_economy()
   wake_me(2000)
   send_msg(story_msg_1)

   wake_me(120000)
   send_msg(briefing_msg_1)
   send_msg(order_msg_1_small_food_economy)

   p1:allow_buildings{"fishers_hut",
      "hunters_hut",
      "gamekeepers_hut",
      "tavern",
      "farm",
      "well",
      "bakery",
   }
   local o = add_obj(obj_build_small_food_economy)
   while not check_for_buildings(p1, {
         fishers_hut = 1,
         hunters_hut = 1,
         well = 1,
         farm = 1,
         bakery = 1,
      }) do sleep(3412) end
   o.done = true
   send_msg(story_note_1)

   sleep(600000)  -- 10 minutes
   remember_cattlefarm()
end

function foottracks()
   -- Hunter build and some time passed or expanded east
   local game = wl.Game()
   while true do
      if (game.time > 900000 and #p1:get_buildings("hunters_hut") > 0)
         or p1:seen_field(wl.Game().map:get_field(65, 28))
      then break end
      sleep(4239)
   end

   local fields = array_combine(
      wl.Game().map:get_field(67, 19):region(2),
      wl.Game().map:get_field(65, 19):region(2),
      wl.Game().map:get_field(69, 18):region(2)
   )
   p1:reveal_fields(fields)

   local pts = scroll_smoothly_to(wl.Game().map:get_field(67,19))

   send_msg(order_msg_2_build_a_donjon)
   local o = add_obj(obj_build_a_donjon)
   p1:forbid_buildings{"sentry"}
   p1:allow_buildings{"donjon"}

   timed_scroll(array_reverse(pts), 10)

   -- Hide the tracks again
   sleep(5000)
   p1:hide_fields(fields)

   while not check_for_buildings(p1, {donjon=1}) do sleep(2341) end
   o.done = true
   send_msg(order_msg_3_explore_further)
   o = add_obj(obj_explore_further)

   p1:allow_buildings{"sentry", "barrier"}

   -- Wait till we see the mountain and at least one frontier
   while true do
      if p1:seen_field(mountain) and
            (p1:seen_field(fr1) or p1:seen_field(fr2) or p1:seen_field(fr3))
      then
         break
      end

      sleep(3244)
   end
   o.done = true
end

function mining_and_trainingsites()
   while not p1:seen_field(mountain) do sleep(7834) end

   -- Show the other mountains permanently
   p1:reveal_fields(array_combine(
      wl.Game().map:get_field(77, 98):region(7),
      wl.Game().map:get_field(79, 6):region(5),
      wl.Game().map:get_field(82, 20):region(6))
   )

   local pts = scroll_smoothly_to(wl.Game().map:get_field(82,20))

   send_msg(order_msg_4_build_mining_economy)
   local o = add_obj(obj_build_mining_economy)

   p1:allow_buildings{
      "coalmine",
      "oremine",
      "goldmine",
      "granitemine",
      "smelting_works",
      "metalworks",
      "burners_house",
   }

   timed_scroll(array_reverse(pts), 10)
   sleep(500)

   while true do
      local h = p1:get_buildings{"coalmine","burners_house", "oremine",
         "tavern", "smelting_works", "metalworks"}

      if (#h.coalmine + #h.burners_house > 0) and
         #h.oremine > 0 and #h.tavern > 0 and #h.smelting_works > 0 and
         #h.metalworks > 0
      then
         break
      end
      sleep(4139)
   end
   o.done = true
   send_msg(story_note_2)

   sleep(100000)

   send_msg(order_msg_6_build_enhanced_economy_and_training)
   p1:allow_buildings{
      "axfactory",
      "warmill",
      "helmsmithy",
      "battlearena",
      "trainingcamp",
      "inn",
      "big_inn",
      "deep_coalmine",
      "deep_goldmine",
      "deep_oremine",
      "deeper_coalmine",
      "deeper_goldmine",
      "deeper_oremine",
      "warehouse",
      "micro-brewery",
      "brewery",
   }
   run(check_warehouse_obj, add_obj(obj_build_a_warehouse))
   run(check_trainingssite_obj, add_obj(obj_build_trainingssites))
   run(check_weapon_productions_obj, add_obj(obj_build_weapon_productions))
   run(check_helmsmithy_obj, add_obj(obj_build_a_helmsmithy))
end

function check_trainingssite_obj(o)
   while not check_for_buildings(p1, {trainingcamp = 1, battlearena = 1}) do
      sleep(6523)
   end
   o.done = true
end
function check_weapon_productions_obj(o)
   while true do
      local rv = p1:get_buildings{"metalworks", "axfactory", "warmill"}
      if #rv.metalworks > 0 and (#rv.axfactory + #rv.warmill > 0) then
         break
      end
      sleep(6523)
   end
   o.done = true
end
function check_warehouse_obj(o)
   while not check_for_buildings(p1, {warehouse = 1}) do sleep(3827) end
   o.done = true
end
function check_helmsmithy_obj(o)
   while not check_for_buildings(p1, {helmsmithy = 1}) do sleep(3827) end
   o.done = true
end

function fortress()
   while not (p1:seen_field(fr1) or p1:seen_field(fr2) or p1:seen_field(fr3)) do
      sleep(7349)
   end

   send_msg(order_msg_5_build_a_fortress)
   local o = add_obj(obj_build_a_fortress)
   p1:allow_buildings{"fortress"}

   while #p1:get_buildings("fortress") == 0 do sleep(6523) end

   o.done = true
   send_msg(story_note_3)
end

function expansion()
   -- While enemy has not been seen
   while not (
      p1:seen_field(wl.Game().map:get_field(95, 91)) or
      p1:seen_field(wl.Game().map:get_field(96, 107)) or
      p1:seen_field(wl.Game().map:get_field(96, 8)) or
      p1:seen_field(wl.Game().map:get_field(96, 19))
      )
   do sleep(8374) end

   send_msg(story_msg_2)
   send_msg(story_msg_3)
   send_msg(story_msg_4)

   sleep(20000)
   send_msg(story_msg_5)
   send_msg(story_msg_6)
end

function kalitath()
   -- While no contact with kalithat
   local map = wl.Game().map
   while not (
      p1:seen_field(map:get_field( 92,  91)) or
      p1:seen_field(map:get_field(102, 103)) or
      p1:seen_field(map:get_field(103,   0)) or
      p1:seen_field(map:get_field(103,  11)) or
      p1:seen_field(map:get_field(103,  17)) or
      p1:seen_field(map:get_field( 96,  30))
      )
   do sleep(7834) end

   send_msg(order_msg_7_destroy_kalitaths_army)
   local o = add_obj(obj_destroy_kalitaths_army)

   while not p2.defeated do sleep(7837) end
   o.done = true
end

function renegade_fortresses()
   -- Wait till we see the fortresses
   local map = wl.Game().map
   while not (
      p1:seen_field(map:get_field(111, 88 )) or
      p1:seen_field(map:get_field(110, 97 )) or
      p1:seen_field(map:get_field(111, 110)) or
      p1:seen_field(map:get_field(111, 7  )) or
      p1:seen_field(map:get_field(114, 14 )) or
      p1:seen_field(map:get_field(116, 21 ))
      )
   do sleep(6834) end

   prefilled_buildings(p1,
      {"barrier", 118, 100, soldiers =
         {[{0,0,0,0}]= 1, [{1,1,0,1}] = 1, [{0,1,0,0}] = 1}
      },
      {"citadel", 117, 97, soldiers = {
         [{0,0,0,0}]=3, [{1,1,0,0}]=1, [{0,0,0,1}]=2,
         [{0,1,0,1}]=1, [{1,1,0,1}]=1, [{2,1,0,2}]=1,
      }},
      {"sentry", 116, 98 }
   )

   -- Some something of the enemy land
   p1:reveal_fields(map:get_field(129,97):region(12))

   local pts = scroll_smoothly_to(map:get_field(120,92))

   send_msg(order_msg_7_renegade_fortification)
   send_msg(order_msg_7_free_althunran)
   local o = add_obj(obj_military_assault_on_althunran)

   timed_scroll(array_reverse(pts))
   sleep(500)

   while not (p3.defeated and p4.defeated) do
      sleep(6734)
   end

   o.done = true
end

function mission_complete()
   local map = wl.Game().map
   while not (p2.defeated and p3.defeated and p4.defeated) do
      sleep(8923)
   end

   p1:reveal_fields(map:get_field(4,9):region(6))

   local pts = scroll_smoothly_to(map:get_field(4,5))

   send_msg(story_msg_7)

   p1:reveal_campaign("campsect1")
end

run(initial_message_and_small_food_economy)
run(foottracks)
run(fortress)
run(expansion)
run(kalitath)
run(renegade_fortresses)
run(mission_complete)
run(mining_and_trainingsites)

