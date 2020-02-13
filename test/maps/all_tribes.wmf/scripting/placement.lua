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

-- Add a building with coordinates not going out of range
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

function place_initial_militarysites(map, sf, player, buildingname)
   -- Left
   local building = place_militarysite(player, buildingname, sf, 506, 0)
   connected_road(player, sf.immovable.flag, "l,l|l,l|l,l")

   -- Right
   building = place_militarysite(player, buildingname, sf, 6, 0)
   connected_road(player, sf.immovable.flag, "r,r|r,r|r,r")

   -- Bottom
   connected_road(player, sf.immovable.flag, "bl,bl|bl,br|bl,br|br,bl,bl")
   building = place_militarysite(player, buildingname, sf, 511, 9)

   -- Mountain
   building = place_militarysite(player, buildingname, sf, 504, 14)
   connected_road(player, building.flag, "tr,tr,r|r,r|r,tr|tr,tr")

   building = place_militarysite(player, buildingname, sf, 499, 509)
   connected_road(player, building.flag, "r,r,r|br,br|br,r|r,r")

   building = place_militarysite(player, buildingname, sf, 495, 14)
   connected_road(player, building.flag, "r,r,r|r,r|r,r|r,r")

   building = place_militarysite(player, buildingname, sf, 492, 1)
   connected_road(player, building.flag, "tr,r,tr|r,r|tr,tr|r,r")

   -- Water
   building = place_militarysite(player, buildingname, sf, 13, 504)
   connected_road(player, building.flag, "bl,bl,bl|bl,bl|bl,bl|bl,l|l,l")

   building = place_militarysite(player, buildingname, sf, 5, 18)
   connected_road(player, building.flag, "l,l,tl|tl,tl|tl,tl|tl,tl|tl,tl")

   building = place_militarysite(player, buildingname, sf, 21, 509)
   connected_road(player, building.flag, "tr,tr|tl,l|l,tl|l,l|tl,l|l,l")

   building = place_militarysite(player, buildingname, sf, 12, 19)
   connected_road(player, building.flag, "l,l,l|l,tl|l,l")

   building = place_militarysite(player, buildingname, sf, 20, 18)
   connected_road(player, building.flag, "l,l,l|l,l|l,l|l,bl")

   building = place_militarysite(player, buildingname, sf, 26, 4)
   connected_road(player, building.flag, "tr,tr,tl|tl,tl|tl,tl|l,tl|l,tl")
end

