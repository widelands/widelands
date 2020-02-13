-- ================================================
--       Scenario with all tribes on the map
-- ================================================


include "scripting/lunit.lua"
include "scripting/infrastructure.lua"

include "map:scripting/placement.lua"
include "map:scripting/barbarians.lua"

-- Check that all buildings have been built
function verify_buildings(player, total_expected_buildings)
  print("============================================")
  print("Checking buildings for tribe: " .. player.tribe.name)
  local total_buildings = 0
  -- Verify that we have placed all buildings
   for idx, building in ipairs(player.tribe.buildings) do
      -- We expect at least 1 building placed of each type.
      -- We can't create dismantlesites or check for constructionsites yet, so we skip them for the check.
      if building.name ~= "constructionsite"
         and building.name ~= "dismantlesite"
         and #player:get_buildings(building.name) < 1 then
         print("Building '" .. building.name .. " hasn't been built yet.")
      end
      total_buildings = total_buildings + #player:get_buildings(building.name)
   end
   print("Found " .. total_buildings .. " buildings")
   if total_buildings ~= total_expected_buildings then
      print("Number of missing buildings: " .. (total_expected_buildings - total_buildings))
   end
   print("============================================")
end

local player = wl.Game().players[1]
player.see_all = 1

init_barbarians(player)
verify_buildings(player, 123)

player = wl.Game().players[2]
player.see_all = 1

init_barbarians(player)
verify_buildings(player, 123)

player = wl.Game().players[3]
player.see_all = 1

init_barbarians(player)
verify_buildings(player, 123)

player = wl.Game().players[4]
player.see_all = 1

init_barbarians(player)
verify_buildings(player, 123)
