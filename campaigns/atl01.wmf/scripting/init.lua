-- =======================================================================
--                      Atlanteans Tutorial Mission 01                      
-- =======================================================================

use("aux", "coroutine")
use("aux", "array")
use("aux", "infrastructure")
use("aux", "objective_utils")
use("aux", "ui")

use("map", "water_rising")

-- ===================
-- Constants & Config
-- ===================
set_textdomain("scenario_atl01.wmf")

game = wl.Game()
map = game.map
p1 = game.players[1]
first_tower_field = map:get_field(94, 149)
second_tower_field = map:get_field(79, 150)

use("map", "texts")

-- =================
-- Helper functions 
-- =================
-- Show one message box
function msg_box(i)
   if i.field then
      scroll_smoothly_to(i.field.trn.trn.trn.trn)

      i.field = nil -- Otherwise message box jumps back
   end

   p1:message_box(i.title, i.body, i)

   sleep(130)
end

-- Show many message boxes
function msg_boxes(boxes_descr)
   for idx,box_descr in ipairs(boxes_descr) do
      msg_box(box_descr)
   end
end

-- Add an objective
function add_obj(o)
   return p1:add_objective(o.name, o.title, o.body)
end

-- Return the total number of items in warehouses of the given
-- ware.
function count_in_warehouses(ware)
   local whs = array_concat(
      player1:get_buildings("headquarters"),
      player1:get_buildings("warehouse")
   )
   rv = 0
   for idx,wh in ipairs(whs) do
      rv = rv + wh:get_ware(wares)
   end
   return rv
end

-- ===============
-- Initialization
-- ===============
function initialize()
   p1:allow_buildings("all")

   -- A default headquarters
   use("tribe_atlanteans", "sc00_headquarters_medium")
   init.func(p1) -- defined in sc00_headquarters_medium
   set_textdomain("scenario_atl01.wmf")

   -- Place some buildings
   prefilled_buildings(p1,
      {"high_tower", first_tower_field.x, first_tower_field.y, 
         soldiers = { [{0,0,0,0}] = 1 }
      },
      {"high_tower", second_tower_field.x, second_tower_field.y, 
         soldiers = { [{0,0,0,0}] = 1 }
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

   initialize()
   
   build_environment()
end

function build_heavy_industries_and_mining()
   -- TODO: make this
end

function build_food_environment()
   msg_boxes(food_story_message)

   run(function()
      sleep(60000)
      run(build_heavy_industries_and_mining)
   end)

   local o = add_obj(obj_make_food_infrastructure)
   while not check_for_buildings(p1, {
      farm = 1, blackroot_farm = 1,
      sawmill = 1, well = 1, bakery = 1,
      hunters_house = 1, fishers_house = 1,
      fish_breeders_house = 1, smokery = 2,
   }) do sleep(2789) end

   msg_boxes(food_story_ended_messages)
end

function make_spidercloth_production()
   local scloth = 0
   while true do
      for bname,buildings in
            pairs(p1:get_buildings{"headquarters", "warehouse"}) do
         for idx,b in ipairs(buildings) do
            scloth += b:get_wares("spidercloth")
         end
      end
      if scloth == 0 then break end
   end

   -- There is no spidercloth in any warehouse!
   message_box(spidercloth_messages)
   local o1 = add_obj(obj_spidercloth_production)

   while not check_for_buildings(p1, {
      spiderfarm = 1, goldweaver = 1, ["weaving-mill"] = 1
   }) do sleep(6273) end
   o1.done = true

   msg_boxes(spidercloth_story_ended_messages)

end

function build_environment()
   msg_boxes(first_briefing_messages)
   local o = add_obj(obj_ensure_build_wares_production)
   -- TODO: o1 is never marked as completed
   local o1 = add_obj(obj_expand)
   
   while not check_for_buildings(p1, {
      woodcutters_house = 3,
      foresters_house = 3,
      quarry = 1,
      sawmill = 1,
   }) do sleep(3731) end
   o.done = true

   run(make_spidercloth_production)

   sleep(45000) -- Sleep a while
   run(build_food_environment)
end

function leftover_buildings()
   -- All fields with left over buildings
   local lob_fields = Set:new{
      map:get_field( 59, 86),
      map:get_field( 60,151),
      map:get_field( 72, 89),
      map:get_field(111,137),
      map:get_field(121,144),
   }

   local msgs = {
      first_leftover_building_found,
      second_leftover_building_found,
      third_leftover_building_found,
   }

   while lob_fields.size > 0 and #msgs > 0 do
      for f in lob_fields:items() do
         if p1:sees_field(f) then
            scroll_smoothly_to(f)

            msg_boxes(msgs[1])

            table.remove(msgs, 1)
            lob_fields:discard(f)
            break
         end
      end
      sleep(3000)
   end
end


wr = WaterRiser:new(map:get_field(92,19))

-- TODO: remove/reuse this debug function 
function start_water_rising()
   -- TODO: remove this again
   p1.see_all = 1

   wr:rise(25) -- Rise the water to level 25
end
function swr() start_water_rising() end

run(intro)
run(leftover_buildings)



