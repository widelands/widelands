-- ================================================
--       Scenario with all tribes on the map
-- ================================================

-- This scenario has been prepared for 8 players.
-- To add a new tribe, copy barbarians.lua into a new file and change all the bulding names.
-- Don't forget to define the tribe in player_names.
-- Make sure that the building sizes are the same, and that ports go on port spaces etc.
-- Then add the checks to the bottom of this file.

-- TODO(GunChleoc): Place waterways when we have the Lua interface for them

include "scripting/coroutine.lua"
include "scripting/lunit.lua"
include "scripting/infrastructure.lua"
include "test/scripting/stable_save.lua"

include "map:scripting/placement.lua"
include "map:scripting/barbarians.lua"
include "map:scripting/empire.lua"
include "map:scripting/atlanteans.lua"
include "map:scripting/frisians.lua"

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

-- See all so that we can debug stuff
wl.Game().players[1].see_all = 1

-- Place buildings
print("Placing buildings for Player 1")
init_barbarians(wl.Game().players[1])

print("Placing buildings for Player 2")
init_empire(wl.Game().players[2])

print("Placing buildings for Player 3")
init_atlanteans(wl.Game().players[3])

print("Placing buildings for Player 4")
init_frisians(wl.Game().players[4])

-- These slots have been prepared for future tribes
-- print("Placing buildings for Player 5")
-- init_barbarians(wl.Game().players[5])

-- print("Placing buildings for Player 6")
-- init_barbarians(wl.Game().players[6])

-- print("Placing buildings for Player 7")
-- init_barbarians(wl.Game().players[7])

-- print("Placing buildings for Player 8")
-- init_barbarians(wl.Game().players[8])

-- Verify that nothing went wrong with placing the buildings,
-- and that a building of each type has been placed.
verify_buildings(1, 122)
verify_buildings(2, 122)
verify_buildings(3, 122)
verify_buildings(4, 122)

-- These slots have been prepared for future tribes
-- verify_buildings(5, 122)
-- verify_buildings(6, 122)
-- verify_buildings(7, 122)
-- verify_buildings(8, 122)



-- Run the tests
run(function()
   sleep(1000)

   -- Test ships
   place_player_ship(1)
   place_player_ship(2)
   place_player_ship(3)
   place_player_ship(4)

   -- Sleep a bit
   sleep(1000)

   -- Test saveloading nd verify the buildings again
   stable_save(game, "all_tribes_buildings")

   verify_buildings(1, 122)
   verify_buildings(2, 122)
   verify_buildings(3, 122)
   verify_buildings(4, 122)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
