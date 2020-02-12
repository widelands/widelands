-- Add a warehouse that has everything in it
function place_warehouse(player, field, tribe, buildingname)
   local building = player:place_building(buildingname, field, false, true)

   -- Add all wares
   wares = {}
   for i, ware in ipairs(tribe.wares) do
      wares[ware.name] = 20
   end
   building:set_wares(wares)

   -- Add all workers
   workers = {}
   for i, worker in ipairs(tribe.workers) do
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
   local building = player:place_building(buildingname, map:get_field(sf.x + 506, sf.y), false, true)
   building:set_soldiers({ [{0,0,0,0}] = 1 })
   connected_road(player, sf.immovable.flag, "l,l|l,l|l,l")

   -- Right
   building = player:place_building(buildingname, map:get_field(sf.x + 6, sf.y), false, true)
   building:set_soldiers({ [{0,0,0,0}] = 1 })
   connected_road(player, sf.immovable.flag, "r,r|r,r|r,r")

   -- Bottom
   connected_road(player, sf.immovable.flag, "bl,bl|bl,br|bl,br|br,bl,bl")
   building = player:place_building(buildingname, map:get_field(sf.x + 511, sf.y + 9), false, true)

   -- Mountain
   building = player:place_building(buildingname, map:get_field(sf.x + 504, sf.y + 14), false, true)
   building:set_soldiers({ [{0,0,0,0}] = 1 })
   connected_road(player, building.flag, "tr,tr,r|r,r|r,tr|tr,tr")

   building = player:place_building(buildingname, map:get_field(sf.x + 499, sf.y + 509), false, true)
   building:set_soldiers({ [{0,0,0,0}] = 1 })
   connected_road(player, building.flag, "r,r,r|br,br|br,r|r,r")

   building = player:place_building(buildingname, map:get_field(sf.x + 495, sf.y + 14), false, true)
   building:set_soldiers({ [{0,0,0,0}] = 1 })
   connected_road(player, building.flag, "r,r,r|r,r|r,r|r,r")

   building = player:place_building(buildingname, map:get_field(sf.x + 492, sf.y + 1), false, true)
   building:set_soldiers({ [{0,0,0,0}] = 1 })
   connected_road(player, building.flag, "tr,r,tr|r,r|tr,tr|r,r")

   -- Water
   building = player:place_building(buildingname, map:get_field(sf.x + 13, sf.y + 504), false, true)
   building:set_soldiers({ [{0,0,0,0}] = 1 })
   connected_road(player, building.flag, "bl,bl,bl|bl,bl|bl,bl|bl,l|l,l")

   building = player:place_building(buildingname, map:get_field(sf.x + 5, sf.y + 18), false, true)
   building:set_soldiers({ [{0,0,0,0}] = 1 })
   connected_road(player, building.flag, "l,l,tl|tl,tl|tl,tl|tl,tl|tl,tl")

   building = player:place_building(buildingname, map:get_field(sf.x + 21, sf.y + 509), false, true)
   building:set_soldiers({ [{0,0,0,0}] = 1 })
   connected_road(player, building.flag, "tr,tr|tl,l|l,tl|l,l|tl,l|l,l")

   building = player:place_building(buildingname, map:get_field(sf.x + 12, sf.y + 19), false, true)
   building:set_soldiers({ [{0,0,0,0}] = 1 })
   connected_road(player, building.flag, "l,l,l|l,tl|l,l")

   building = player:place_building(buildingname, map:get_field(sf.x + 20, sf.y + 18), false, true)
   building:set_soldiers({ [{0,0,0,0}] = 1 })
   connected_road(player, building.flag, "l,l,l|l,l|l,l|l,bl")

   building = player:place_building(buildingname, map:get_field(sf.x + 26, sf.y + 4), false, true)
   building:set_soldiers({ [{0,0,0,0}] = 1 })
   connected_road(player, building.flag, "tr,tr,tl|tl,tl|tl,tl|l,tl|l,tl")
end
