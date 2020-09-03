-- ================================================
--       Scenario with all tribes on the map
-- ================================================

-- This scenario has been prepared for 8 players.
-- To add a new tribe, copy test_barbarians.lua into a new file and change all the bulding names.
-- Don't forget to define the tribe in player_names.
-- Make sure that the building sizes are the same or smaller, and that ports go on port spaces etc.

include "scripting/coroutine.lua"
include "scripting/lunit.lua"
include "scripting/infrastructure.lua"
include "test/scripting/stable_save.lua"

game = wl.Game()

-- See all so that we can debug stuff
game.players[1].see_all = 1
mapview = wl.ui.MapView()

-- Check that all buildings have been built
function verify_buildings(playernumber, total_expected_buildings)
  local player = wl.Game().players[playernumber]
  print("============================================")
  print("Checking buildings for tribe: " .. player.tribe.name)
  print("Player: " .. playernumber)
  local total_buildings = 0
  -- Verify that we have placed all buildings
   for idx, building in ipairs(player.tribe.buildings) do
      -- We expect at least 1 building placed of each type.
      -- We can't create dismantlesites or check for constructionsites yet, so we skip them for the check.
      if building.name ~= "constructionsite" and building.name ~= "dismantlesite" then
         if #player:get_buildings(building.name) < 1 then
            print("Building '" .. building.name .. "' hasn't been built yet.")
         end
         -- This can be commented out for convenience while adding a new tribe.
         --  So, we still keep the message above too.
         assert_true(#player:get_buildings(
            building.name) > 0,
            "Building '" .. building.name .. "' should have been built somewhere.")
      end
      total_buildings = total_buildings + #player:get_buildings(building.name)
   end
   print("Found " .. total_buildings .. " buildings")
   if total_buildings ~= total_expected_buildings then
      -- For scenario debugging. If a few buildings are missing but we still have each building type,
      -- that's good enough for the testing requirement. So, no assert here.
      print("Number of missing buildings: " .. (total_expected_buildings - total_buildings))
   end
   print("============================================")
end


-- Counts all buildings currently owned by the 'player' and
-- deducts 'old_count' to see if the remainder matches 'expected_buildings'
function count_buildings(player, old_count, expected_buildings)
  local total_buildings = 0
  -- Verify that we have placed all buildings
   for idx, building in ipairs(player.tribe.buildings) do
      total_buildings = total_buildings + #player:get_buildings(building.name)
   end
   local actual_buildings = total_buildings - old_count

   print("--------------------------------------------")
   print("Found " .. actual_buildings .. " new buildings")
   if actual_buildings ~= expected_buildings then
      print("Number of missing buildings: " .. (expected_buildings - actual_buildings))
   end
   print("============================================")
   return total_buildings
end


-- Placement functions

-- Get a field with the coordinates shifted for the player
function get_safe_field(player, starting_field, x, y)
   return map:get_field((starting_field.x + x) % 512, (starting_field.y + y) % 512)
end

-- Add a building with coordinates not going out of range.
-- Note that this has only been tested with starting_field.y == 1
function place_safe_building(player, buildingname, starting_field, x, y)
   print("Placing " .. buildingname .. " at " .. ((starting_field.x + x) % 512) .. " " .. ((starting_field.y + y) % 512))
   return player:place_building(buildingname, map:get_field((starting_field.x + x) % 512, (starting_field.y + y) % 512), false, true)
end

-- Place a militarysite and add a soldier to it
function place_militarysite(player, buildingname, starting_field, x, y)
   local building = place_safe_building(player, buildingname, starting_field, x, y)
   building:set_soldiers({ [{0,0,0,0}] = 1 })
   return building
end

-- Add a warehouse that has everything in it
function place_warehouse(player, buildingname, starting_field, x, y)
   local building = place_safe_building(player, buildingname, starting_field, x, y)

   -- Add all wares
   wares = {}
   for i, ware in ipairs(player.tribe.wares) do
      wares[ware.name] = 20
   end
   building:set_wares(wares)

   -- Add all workers
   workers = {}
   for i, worker in ipairs(player.tribe.workers) do
      -- Skip soldiers, they have special code
      if (worker.type_name ~= "soldier") then
         workers[worker.name] = 20
      end
   end
   building:set_workers(workers)

   building:set_soldiers({ [{0,0,0,0}] = 100 })
   return building
end

-- Call this with any big militarysite for your tribe
function place_initial_militarysites(map, sf, player, buildingname)
   -- Left
   local building = place_militarysite(player, buildingname, sf, 506, 0)
   connected_road("normal", player, sf.immovable.flag, "l,l|l,l|l,l")

   -- Right
   building = place_militarysite(player, buildingname, sf, 6, 0)
   connected_road("normal", player, sf.immovable.flag, "r,r|r,r|r,r")

   -- Bottom
   connected_road("normal", player, sf.immovable.flag, "bl,bl|bl,br|bl,br|br,bl,bl")
   building = place_militarysite(player, buildingname, sf, 511, 9)

   -- Mountain
   building = place_militarysite(player, buildingname, sf, 504, 14)
   connected_road("normal", player, building.flag, "tr,tr,r|r,r|r,tr|tr,tr")

   building = place_militarysite(player, buildingname, sf, 499, 509)
   connected_road("normal", player, building.flag, "r,r,r|br,br|br,r|r,r")

   building = place_militarysite(player, buildingname, sf, 495, 14)
   connected_road("normal", player, building.flag, "r,r,r|r,r|r,r|r,r")

   -- Water
   building = place_militarysite(player, buildingname, sf, 13, 504)
   connected_road("normal", player, building.flag, "bl,bl,bl|bl,bl|bl,bl|bl,l|l,l")

   building = place_militarysite(player, buildingname, sf, 5, 18)
   connected_road("normal", player, building.flag, "l,l,tl|tl,tl|tl,tl|tl,tl|tl,tl")

   building = place_militarysite(player, buildingname, sf, 21, 509)
   connected_road("normal", player, building.flag, "tr,tr|tl,l|l,tl|l,l|tl,l|l,l")

   building = place_militarysite(player, buildingname, sf, 12, 19)
   connected_road("normal", player, building.flag, "l,l,l|l,tl|l,l")

   building = place_militarysite(player, buildingname, sf, 20, 18)
   connected_road("normal", player, building.flag, "l,l,l|l,l|l,l|l,bl")

   building = place_militarysite(player, buildingname, sf, 26, 4)
   connected_road("normal", player, building.flag, "tr,tr,tl|tl,tl|tl,tl|l,tl|l,tl")
end

function place_player_ship(playernumber)
   local player = wl.Game().players[playernumber]
   local starting_field = wl.Game().map.player_slots[playernumber].starting_field
   player:place_ship(map:get_field((starting_field.x + 12) % 512, (starting_field.y + 6) % 512))
end

-- Sleep and adjust game speed each second for reasonable average FPS
function sleep_with_fps(seconds)
   local counter = 0
   repeat
      counter = counter + 1
      sleep(1000)
      local average_fps = mapview.average_fps
      if average_fps < 15 then
         local new_desired_speed = game.desired_speed - 1000 * (20 - math.floor(average_fps))
         if new_desired_speed < 5000 then new_desired_speed = 5000 end
         game.desired_speed = new_desired_speed
      elseif average_fps > 20 then
         game.desired_speed = game.desired_speed + 1000
      end
   until counter == seconds
end
