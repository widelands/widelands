-- ================================================
--       Scenario with all tribes on the map
-- ================================================


-- include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"

include "map:scripting/placement.lua"
include "map:scripting/barbarians.lua"

function verify_buildings(player)
  print("Start checking buildings for tribe: " .. player.tribe.name)
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
   print("Done checking buildings for tribe: " .. player.tribe.name)
   -- Temp to check the layout. Expect Found 123 buildings
   print("Found " .. total_buildings .. " buildings")
end

local player = wl.Game().players[1]
player.see_all = 1

init_barbarians(player)
verify_buildings(player)

player = wl.Game().players[2]
player.see_all = 1

init_barbarians(player)
verify_buildings(player)
