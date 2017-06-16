-- =======================================================================
--                          Various mission threads
-- =======================================================================

include "scripting/messages.lua"
include "scripting/field_animations.lua"

game = wl.Game()
-- Mountain and frontier fields
mountain = game.map:get_field(71,14)
fr1 = game.map:get_field(81,108)
fr2 = game.map:get_field(85,1)
fr3 = game.map:get_field(85,11)

-- Starting field
sf = game.map.player_slots[1].starting_field

function check_conquered_footprints()
    if p1:seen_field(game.map:get_field(65, 28))
    then
        sleep(2129)
        if p1:sees_field(game.map:get_field(65, 28))
        then return true end
    end
    return false
end

function remember_cattlefarm()
   sleep(100)

   campaign_message_box(cattlefarm_01)

   p1:allow_buildings{"barbarians_cattlefarm"}
   local o = add_campaign_objective(obj_build_cattlefarm)
   while not check_for_buildings(p1, {barbarians_cattlefarm = 1}) do
      sleep(1223) end
   set_objective_done(o)

end

function initial_message_and_small_food_economy()
   reveal_concentric(p1, sf, 14)
   wake_me(2000)
   campaign_message_box(story_msg_1)

   local o = add_campaign_objective(obj_build_basic_economy)
   while not check_for_buildings(p1, {
         barbarians_lumberjacks_hut = 1,
         barbarians_rangers_hut = 1,
         barbarians_quarry = 1,
         barbarians_wood_hardener = 1,
      }) do sleep(3413) end
   while not check_for_buildings(p1, {barbarians_sentry=1}, game.map:get_field(57,36):region(6)) do
      sleep(1500)
   end
   set_objective_done(o)
   sleep(3000)

   campaign_message_box(briefing_msg_1)
   campaign_message_box(order_msg_1_small_food_economy)

   p1:allow_buildings{
      "barbarians_fishers_hut",
      "barbarians_hunters_hut",
      "barbarians_gamekeepers_hut",
      "barbarians_tavern",
      "barbarians_farm",
      "barbarians_well",
      "barbarians_bakery",
      "barbarians_sentry",
   }

   o = add_campaign_objective(obj_build_small_food_economy)
   while not check_for_buildings(p1, {
         barbarians_fishers_hut = 1,
         barbarians_hunters_hut = 1,
         barbarians_well = 1,
         barbarians_farm = 1,
         barbarians_bakery = 1,
      }) do sleep(3413) end
   set_objective_done(o)
   campaign_message_box(story_note_1)

   sleep(600000)  -- 10 minutes
   remember_cattlefarm()
end

function foottracks()
   -- Hunter build and some time passed or expanded east
   while true do
      if game.time > 900000 and #p1:get_buildings("barbarians_hunters_hut") > 0
      then break end
      if check_conquered_footprints()
      then break end
      sleep(4239)
   end

   local fields = array_combine(
      game.map:get_field(67, 19):region(2),
      game.map:get_field(65, 19):region(2),
      game.map:get_field(69, 18):region(2)
   )

   p1:hide_fields(fields)
   local prior_center = scroll_to_field(game.map:get_field(65,19))
   -- reveal the tracks one by one from right to left
   sleep(1000)
   reveal_concentric(p1, game.map:get_field(69, 19),2, false)
   sleep(500)
   reveal_concentric(p1, game.map:get_field(67, 19),2, false)
   sleep(500)
   reveal_concentric(p1, game.map:get_field(65, 19),2, false)
   sleep(1000)
   campaign_message_box(order_msg_2_build_a_tower)
   local o = add_campaign_objective(obj_build_a_tower)
   p1:forbid_buildings{"barbarians_sentry"}
   p1:allow_buildings{"barbarians_tower"}

   scroll_to_map_pixel(prior_center)

   -- Hide the tracks again
   sleep(5003)
   p1:hide_fields(fields)

   while not check_for_buildings(p1, {barbarians_tower=1}) do sleep(2341) end
   set_objective_done(o)
   campaign_message_box(order_msg_3_explore_further)
   -- "explore further" is active untill "Boldreth shout out", so the player always has one open objectve.
   exploration_objective = add_campaign_objective(obj_explore_further)

   p1:allow_buildings{"barbarians_sentry", "barbarians_barrier"}

   -- Wait till we see the mountain and at least one frontier
   while true do
      if p1:seen_field(mountain) and
            (p1:seen_field(fr1) or p1:seen_field(fr2) or p1:seen_field(fr3))
      then
         break
      end

      sleep(3244)
   end
end

function mining_and_trainingsites()
   while not p1:seen_field(mountain) do sleep(7834) end

   -- Show the other mountains permanently
   p1:reveal_fields(array_combine(
      game.map:get_field(77, 98):region(7),
      game.map:get_field(79, 6):region(5),
      game.map:get_field(82, 20):region(6))
   )

   local prior_center = scroll_to_field(game.map:get_field(82,20))

   campaign_message_box(order_msg_4_build_mining_economy)
   local o = add_campaign_objective(obj_build_mining_economy)

   p1:allow_buildings{
      "barbarians_coalmine",
      "barbarians_ironmine",
      "barbarians_goldmine",
      "barbarians_granitemine",
      "barbarians_smelting_works",
      "barbarians_metal_workshop",
      "barbarians_charcoal_kiln",
      "barbarians_micro_brewery",
   }

   scroll_to_map_pixel(prior_center)
   sleep(500)

   while true do
      local h = p1:get_buildings{
         "barbarians_coalmine",
         "barbarians_charcoal_kiln",
         "barbarians_ironmine",
         "barbarians_tavern",
         "barbarians_smelting_works",
         "barbarians_metal_workshop"}

      if (#h.barbarians_coalmine + #h.barbarians_charcoal_kiln > 0) and
          #h.barbarians_ironmine > 0 and
          #h.barbarians_tavern > 0 and
          #h.barbarians_smelting_works > 0 and
          #h.barbarians_metal_workshop > 0
      then
         break
      end
      sleep(4139)
   end
   set_objective_done(o)
   campaign_message_box(story_note_2)

   sleep(100000)

   campaign_message_box(order_msg_6_build_enhanced_economy_and_training)
   p1:allow_buildings{
      "barbarians_ax_workshop",
      "barbarians_warmill",
      "barbarians_helmsmithy",
      "barbarians_barracks",
      "barbarians_battlearena",
      "barbarians_trainingcamp",
      "barbarians_inn",
      "barbarians_big_inn",
      "barbarians_coalmine_deep",
      "barbarians_goldmine_deep",
      "barbarians_ironmine_deep",
      "barbarians_coalmine_deeper",
      "barbarians_goldmine_deeper",
      "barbarians_ironmine_deeper",
      "barbarians_warehouse",
      "barbarians_brewery",
   }
   run(check_warehouse_obj, add_campaign_objective(obj_build_a_warehouse))
   run(check_trainingssite_obj, add_campaign_objective(obj_build_trainingssites))
   run(check_weapon_productions_obj, add_campaign_objective(obj_build_weapon_productions))
   run(check_helmsmithy_obj, add_campaign_objective(obj_build_a_helmsmithy))
end

function check_trainingssite_obj(o)
   while not check_for_buildings(p1, {barbarians_barracks = 1, barbarians_trainingcamp = 1, barbarians_battlearena = 1}) do
      sleep(6523)
   end
   set_objective_done(o)
end
function check_weapon_productions_obj(o)
   while true do
      local rv = p1:get_buildings{
         "barbarians_metal_workshop",
         "barbarians_ax_workshop",
         "barbarians_warmill"}
      if #rv.barbarians_metal_workshop > 0 and
         (#rv.barbarians_ax_workshop + #rv.barbarians_warmill > 0) then
         break
      end
      sleep(6523)
   end
   set_objective_done(o)
end
function check_warehouse_obj(o)
   while not check_for_buildings(p1, {barbarians_warehouse = 1}) do sleep(3827) end
   set_objective_done(o)
end
function check_helmsmithy_obj(o)
   while not check_for_buildings(p1, {barbarians_helmsmithy = 1}) do sleep(3827) end
   set_objective_done(o)
end

function fortress()
   while not (p1:seen_field(fr1) or p1:seen_field(fr2) or p1:seen_field(fr3)) do
      sleep(7349)
   end

   campaign_message_box(order_msg_5_build_a_fortress)
   local o = add_campaign_objective(obj_build_a_fortress)
   p1:allow_buildings{"barbarians_fortress"}

   while #p1:get_buildings("barbarians_fortress") == 0 do sleep(6523) end

   set_objective_done(o)
   campaign_message_box(story_note_3)
end

function expansion()
   -- While enemy has not been seen
   while not (
      p1:seen_field(game.map:get_field(95, 91)) or
      p1:seen_field(game.map:get_field(96, 107)) or
      p1:seen_field(game.map:get_field(96, 8)) or
      p1:seen_field(game.map:get_field(96, 19))
      )
   do sleep(8374) end

   campaign_message_box(story_msg_2)
   campaign_message_box(story_msg_3)
   campaign_message_box(story_msg_4)

   sleep(20000)
   campaign_message_box(story_msg_5)
   campaign_message_box(story_msg_6)
end

-- checks if all military buildings are drestroyed (the build-in function "defeated" only checks if all warehouses are destroyed)
function check_player_completely_defeated(dp)
    if #dp:get_buildings("barbarians_fortress")  > 0 then return false end
    if #dp:get_buildings("barbarians_citadel")   > 0 then return false end
    if #dp:get_buildings("barbarians_tower")    > 0 then return false end
    if #dp:get_buildings("barbarians_barrier")   > 0 then return false end
    if #dp:get_buildings("barbarians_sentry")    > 0 then return false end
    if #dp:get_buildings("barbarians_warehouse") > 0 then return false end
    return true
end

function kalitath()
   -- While no contact with kalithat
   local map = game.map
   while not (
      p1:seen_field(map:get_field( 92,  91)) or
      p1:seen_field(map:get_field(102, 103)) or
      p1:seen_field(map:get_field(103,   0)) or
      p1:seen_field(map:get_field(103,  11)) or
      p1:seen_field(map:get_field(103,  17)) or
      p1:seen_field(map:get_field( 96,  30))
      )
   do sleep(7829) end
   -- "explore further" is done
   set_objective_done(exploration_objective)

   campaign_message_box(order_msg_7_destroy_kalitaths_army)
   local o = add_campaign_objective(obj_destroy_kalitaths_army)

   while not check_player_completely_defeated(p2) do
        sleep(7837)
   end
   set_objective_done(o)
end

function renegade_fortresses()
   -- Wait till we see the fortresses
   local map = game.map
   while not (
      p1:seen_field(map:get_field(111, 88 )) or
      p1:seen_field(map:get_field(110, 97 )) or
      p1:seen_field(map:get_field(111, 110)) or
      p1:seen_field(map:get_field(111, 7  )) or
      p1:seen_field(map:get_field(114, 14 )) or
      p1:seen_field(map:get_field(116, 21 ))
      )
   do sleep(6833) end

   prefilled_buildings(p1,
      {"barbarians_barrier", 118, 100, soldiers =
         {[{0,0,0,0}]= 1, [{1,1,0,1}] = 1, [{0,1,0,0}] = 1}
      },
      {"barbarians_citadel", 117, 97, soldiers = {
         [{0,0,0,0}]=3, [{1,1,0,0}]=1, [{0,0,0,1}]=2,
         [{0,1,0,1}]=1, [{1,1,0,1}]=1, [{2,1,0,2}]=1,
      }},
      {"barbarians_sentry", 116, 98 }
   )

   -- Some something of the enemy land
   p1:reveal_fields(map:get_field(129,97):region(12))

   local prior_center = scroll_to_field(map:get_field(120,92))

   campaign_message_box(order_msg_7_renegade_fortification)
   campaign_message_box(order_msg_7_free_althunran)
   local o = add_campaign_objective(obj_military_assault_on_althunran)

   scroll_to_map_pixel(prior_center)
   sleep(503)

   while not (check_player_completely_defeated(p3) and check_player_completely_defeated(p4)) do
      sleep(6733)
   end

   set_objective_done(o)
end

function mission_complete()
   local map = game.map
   while not (
      check_player_completely_defeated(p2) and
      check_player_completely_defeated(p3) and
      check_player_completely_defeated(p4)) do
      sleep(8923)
   end

   p1:reveal_fields(map:get_field(4,9):region(6))

   scroll_to_field(map:get_field(4,5))

   campaign_message_box(story_msg_7)

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
