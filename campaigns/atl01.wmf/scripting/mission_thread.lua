-- =======================================================================
--                      Atlanteans Tutorial Mission 01
-- =======================================================================


-- ==============
-- Logic Threads
-- ==============
function intro()
   sleep(1000)

   msg_boxes(initial_messages)
   sleep(200)

   include "map:scripting/starting_conditions.lua"

   build_environment()
end

function build_warehouse_and_horsefarm()
   local fields = {
      map:get_field(42, 88),
      map:get_field(64, 105),
      map:get_field(93, 89),
      map:get_field(90, 66),
   }

   local fowned = nil
   while not fowned do
      for idx, f in ipairs(fields) do
         if f.owner == p1 then
            fowned = f
            break
         end
      end
      sleep(3213)
   end
   -- Has been started from the very beginning
   expand_objective.done = true
   let_the_water_rise = true

   local pts = wait_for_roadbuilding_and_scroll(fowned)
   msg_boxes(horsefarm_and_warehouse_story)
   -- Go back to where we were
   timed_scroll(array_reverse(pts))

   local o = add_campaign_objective(obj_horsefarm_and_warehouse)
   while not check_for_buildings(p1, {
      horsefarm = 1, warehouse = 1,
   }) do sleep(2384) end
   o.done = true
end

function build_training()
   msg_boxes(training_story)

   local o = add_campaign_objective(obj_make_training_buildings)
   while not check_for_buildings(p1, {
      dungeon = 1, labyrinth = 1
   }) do sleep(3874) end
   o.done = true

   msg_boxes(training_story_end)
end

function build_heavy_industrys_and_mining()
   msg_boxes(heavy_industry_story)

   local o = add_campaign_objective(obj_make_heavy_industry_and_mining)
   while not check_for_buildings(p1, {
      coalmine = 1, ironmine = 1, goldmine = 1, crystalmine = 1,
      smelting_works = 1, weaponsmithy = 1, armorsmithy = 1,
      toolsmithy = 1,
   }) do sleep(3478) end
   o.done = true

   sleep(15 * 60 * 1000) -- sleep a while
   run(build_training)
end

function build_food_environment()
   msg_boxes(food_story_message)

   run(function()
      sleep(10 * 60 * 1000)
      run(build_heavy_industrys_and_mining)
   end)

   local o = add_campaign_objective(obj_make_food_infrastructure)
   while not check_for_buildings(p1, {
      farm = 1, blackroot_farm = 1,
      sawmill = 1, well = 1, bakery = 1,
      hunters_house = 1, fishers_house = 1,
      fish_breeders_house = 1, smokery = 2,
   }) do sleep(2789) end
   o.done = true

   msg_boxes(food_story_ended_messages)
end

function make_spidercloth_production()
   while count_in_warehouses("spidercloth") > 0 do sleep(2323) end

   -- There is no spidercloth in any warehouse!
   msg_boxes(spidercloth_messages)
   local o = add_campaign_objective(obj_spidercloth_production)

   while not check_for_buildings(p1, {
      spiderfarm = 1, ["gold-spinning-mill"] = 1, ["weaving-mill"] = 1
   }) do sleep(6273) end
   o.done = true

   msg_boxes(spidercloth_story_ended_messages)

end

function build_environment()
   msg_boxes(first_briefing_messages)
   local o = add_campaign_objective(obj_ensure_build_wares_production)

   expand_objective = add_campaign_objective(obj_expand)

   while not check_for_buildings(p1, {
      woodcutters_house = 2,
      foresters_house = 2,
      quarry = 1,
      sawmill = 1,
   }) do sleep(3731) end
   o.done = true

   run(make_spidercloth_production)

   sleep(15 * 60 * 1000) -- Sleep a while
   run(build_food_environment)
end

function leftover_buildings()
   -- All fields with left over buildings
   local lob_fields = Set:new{
      map:get_field( 59, 86),
      map:get_field( 72, 91),
      map:get_field(106,102),
      map:get_field(111,137),
      map:get_field(121,144),
      map:get_field(147,121),
   }

   local msgs = {
      first_leftover_building_found,
      second_leftover_building_found,
      third_leftover_building_found,
   }

   while lob_fields.size > 0 and #msgs > 0 do
      for f in lob_fields:items() do
         if p1:sees_field(f) then
            wait_for_roadbuilding_and_scroll(f)

            msg_boxes(msgs[1])

            table.remove(msgs, 1)
            lob_fields:discard(f)
            break
         end
      end
      sleep(3000)
   end
end

function check_for_ships()
   -- Check if the ships are done, then the mission ends successfully
   while #p1:get_ships() < 3 do
        sleep(8234)
   end

   -- Success
   msg_boxes(scenario_won)
   p1:reveal_scenario("atlanteans01")

end


wr = WaterRiser:new(map:get_field(92,19))
function water_rising()
   while not let_the_water_rise do sleep(3243) end

   first_message_send = nil
   local _callback_function = function(f)
      if p1:sees_field(f) and not first_message_send then
         first_message_send = true
         run(function()
            sleep(200)
            wait_for_roadbuilding_and_scroll(f)
            msg_boxes(field_flooded_msg)
            add_campaign_objective(obj_build_ships)
            p1:allow_buildings{"shipyard"}
            run(check_for_ships)
         end)
      end

      if f.owner ~= p1 then return end
      if not f.immovable then return end

      -- Flags are not so interesting
      if f.immovable.descr.type_name == "flag" and
         (f.tln.immovable and is_building(f.tln.immovable)) then
         f = f.tln
      end
      if is_building(f.immovable) then
         send_building_lost_message(f)
      end
   end

   wr.field_flooded_callback = _callback_function

   wr:rise(51)

end

run(intro)
run(leftover_buildings)
run(build_warehouse_and_horsefarm)

run(water_rising)
