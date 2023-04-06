-- =======================================================================
--                      Atlanteans Tutorial Mission 01
-- =======================================================================

function send_building_lost_message(f)
   local message = building_lost(f.immovable.descr.name)
   send_to_inbox(
      p1,
      message.title,
      message.text,
      {
         field = f,
         popup = false,
         icon = f.immovable.descr.icon_name,
         message.title
      }
   )
end

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
   -- Make sure that we can feed the horses at all.
   while not check_for_buildings(p1, {
      atlanteans_farm = 1,
      atlanteans_well = 1,
   }) do sleep(2500) end

   -- Sleeps until one of the given fields is owned by p1.
   local function wait_for_owns_a_field(fields)
      local fowned = nil
      while not fowned do
         for idx, f in ipairs(fields) do
            if f.owner == p1 then
               fowned = f
               break
            end
         end
         sleep(5000)
      end
   end

   wait_for_owns_a_field({
      map:get_field(60, 139),
      map:get_field(98, 128),
      map:get_field(120, 126)
   })

   msg_boxes(horsefarm_and_warehouse_story)
   local o = add_campaign_objective(obj_horsefarm_and_warehouse)
   while not check_for_buildings(p1, {
      atlanteans_horsefarm = 1,
      atlanteans_warehouse = 1,
   }) do sleep(2384) end
   set_objective_done(o)

   -- Now check if the water should rise
   wait_for_owns_a_field({
      map:get_field(42, 88),
      map:get_field(64, 105),
      map:get_field(93, 89),
      map:get_field(90, 66),
   })

   -- Has been started from the very beginning
   set_objective_done(expand_objective)
   let_the_water_rise = true
end

function build_training()
   msg_boxes(training_story)

   local o = add_campaign_objective(obj_make_training_buildings)
   while not check_for_buildings(p1, {
      atlanteans_dungeon = 1,
      atlanteans_labyrinth = 1
   }) do sleep(3874) end
   set_objective_done(o)

   msg_boxes(training_story_end)
end

function toolsmith_hint()
   while not check_for_buildings(p1, {
      atlanteans_toolsmithy = 1,
   }) do sleep(3478) end
   msg_boxes(hint_for_toolsmith)
end

function build_heavy_industrys_and_mining()
   msg_boxes(heavy_industry_story)
   run(toolsmith_hint)

   local o = add_campaign_objective(obj_make_heavy_industry_and_mining)
   while not check_for_buildings(p1, {
      atlanteans_coalmine = 1,
      atlanteans_ironmine = 1,
      atlanteans_goldmine = 1,
      atlanteans_crystalmine = 1,
      atlanteans_smelting_works = 1,
      atlanteans_weaponsmithy = 1,
      atlanteans_armorsmithy = 1,
      atlanteans_toolsmithy = 1,
   }) do sleep(3478) end
   set_objective_done(o)

   run(build_barracks)
end

function build_barracks()

   msg_boxes(barracks_story)

   local o = add_campaign_objective(obj_build_barracks)
   while not check_for_buildings(p1, {
      atlanteans_barracks = 1,
   }) do sleep(3784) end
   set_objective_done(o)

   sleep(1 * 60 * 1000) -- give the recruits time to move
   msg_boxes(barracks_story_end)

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
      atlanteans_farm = 1,
      atlanteans_blackroot_farm = 1,
      atlanteans_sawmill = 1,
      atlanteans_well = 1,
      atlanteans_bakery = 1,
      atlanteans_hunters_house = 1,
      atlanteans_fishers_house = 1,
      atlanteans_fishbreeders_house = 1,
      atlanteans_smokery = 2,
   }) do sleep(2789) end
   set_objective_done(o)

   msg_boxes(food_story_ended_messages)
end

function make_spidercloth_production()
   while count_in_warehouses("spidercloth") > 0 do sleep(2323) end

   -- There is no spidercloth in any warehouse!
   msg_boxes(spidercloth_messages)
   local o = add_campaign_objective(obj_spidercloth_production)

   while not check_for_buildings(p1, {
      atlanteans_spiderfarm = 1,
      atlanteans_gold_spinning_mill = 1,
      atlanteans_weaving_mill = 1
   }) do sleep(6273) end
   set_objective_done(o)

   msg_boxes(spidercloth_story_ended_messages)

end

function build_environment()
   msg_boxes(first_briefing_messages)
   local o = add_campaign_objective(obj_ensure_build_wares_production)
   -- TODO(Nordfriese): Re-add training wheels code after v1.0
   -- p1:run_training_wheel("objectives", false)

   expand_objective = add_campaign_objective(obj_expand)

   while not check_for_buildings(p1, {
      atlanteans_woodcutters_house = 2,
      atlanteans_foresters_house = 2,
      atlanteans_quarry = 1,
      atlanteans_sawmill = 1,
   }) do sleep(3731) end
   set_objective_done(o)

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

   -- The next scenario starts with these ships, so we save their names for continuity
   local persist = { shipnames = {} }
   for i,ship in ipairs(p1:get_ships()) do
      persist.shipnames[i] = ship.shipname
   end
   wl.Game():save_campaign_data("atlanteans", "atl01", persist)

   -- Success
   msg_boxes(scenario_won)
   p1:mark_scenario_as_solved("atl01.wmf")
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
            p1:allow_buildings{"atlanteans_shipyard"}
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
