-- ===============
-- Initialization
-- ===============

function init_barbarians(player)


   local tribe = player.tribe
   -- NOCOM assert that tribe is barbarians
   local sf = wl.Game().map.player_slots[player.number].starting_field
   map = wl.Game().map

   -- Headquarters
   place_warehouse(player, sf, tribe, "barbarians_headquarters")

   -- Get some territory
   place_initial_militarysites(map, sf, player, "barbarians_fortress")

   -- Military small
   player:place_building("barbarians_scouts_hut", map:get_field(sf.x + 21, sf.y + 16), false, true)
   player:place_building("barbarians_scouts_hut", map:get_field(sf.x + 13, sf.y + 18), false, true)

   player:place_building("barbarians_sentry", map:get_field(sf.x + 27, sf.y + 8), false, true)
   player:place_building("barbarians_sentry", map:get_field(sf.x + 31, sf.y + 3), false, true)
   player:place_building("barbarians_sentry", map:get_field(sf.x + 33, sf.y + 3), false, true)
   player:place_building("barbarians_sentry", map:get_field(sf.x + 35, sf.y + 3), false, true)
   player:place_building("barbarians_sentry", map:get_field(sf.x + 27, sf.y + 507), false, true)
   player:place_building("barbarians_sentry", map:get_field(sf.x + 24, sf.y + 501), false, true)

   -- Military medium
   player:place_building("barbarians_barrier", map:get_field(sf.x + 9, sf.y + 499), false, true)
   player:place_building("barbarians_barrier", map:get_field(sf.x + 6, sf.y + 28), false, true)
   player:place_building("barbarians_barrier", map:get_field(sf.x + 509, sf.y + 23), false, true)

   player:place_building("barbarians_tower", map:get_field(sf.x + 486, sf.y + 7), false, true)
   player:place_building("barbarians_tower", map:get_field(sf.x + 491, sf.y + 23), false, true)
   player:place_building("barbarians_tower", map:get_field(sf.x + 26, sf.y + 11), false, true)

   -- Military big
   player:place_building("barbarians_citadel", map:get_field(sf.x + 503, sf.y + 500), false, true)
   player:place_building("barbarians_citadel", map:get_field(sf.x + 11, sf.y + 28), false, true)
   player:place_building("barbarians_citadel", map:get_field(sf.x + 507, sf.y + 26), false, true)

   -- Ports, and warehouses
   place_warehouse(player, map:get_field(sf.x + 11, sf.y + 509), tribe, "barbarians_port")
   building = place_warehouse(player, map:get_field(sf.x + 18, sf.y + 13), tribe, "barbarians_port")
   connected_road(player, building.flag, "br,bl,bl|br,bl")

   building = place_warehouse(player, map:get_field(sf.x + 488, sf.y + 6), tribe, "barbarians_warehouse")
   connected_road(player, building.flag, "tr,tr|r,tr|tr,tr")
   connected_road(player, building.flag, "br,br,br|br,br|br,br|br,r|r,r")

   building = place_warehouse(player, map:get_field(sf.x + 24, sf.y + 10), tribe, "barbarians_warehouse")
   connected_road(player, building.flag, "tr,tr|tr,tr|tl,tr")
   connected_road(player, building.flag, "bl,bl|bl,bl|bl,bl|bl,bl")

   building = place_warehouse(player, map:get_field(sf.x + 11, sf.y + 495), tribe, "barbarians_headquarters_interim")
   connected_road(player, building.flag, "br,br,br|br,br|br,br|br,br")

   -- Shipyards and ferry yards
   building = player:place_building("barbarians_shipyard", map:get_field(sf.x + 14, sf.y + 16), false, true)
   connected_road(player, building.flag, "bl,bl")

   building = player:place_building("barbarians_shipyard", map:get_field(sf.x + 8, sf.y), false, true)

   building = player:place_building("barbarians_ferry_yard", map:get_field(sf.x + 8, sf.y + 12), false, true)
   connected_road(player, building.flag, "bl,bl|bl,bl|bl,bl")

   building = player:place_building("barbarians_ferry_yard", map:get_field(sf.x + 18, sf.y + 1), false, true)
   connected_road(player, building.flag, "tr,tr,tr|tr,r")

   -- Building materials small
   -- (near rocks)
   building = player:place_building("barbarians_quarry", map:get_field(sf.x + 508, sf.y + 502), false, true)
   connected_road(player, building.flag, "r,r,r")
   connected_road(player, building.flag, "br,br|br,br,r")

   building = player:place_building("barbarians_quarry", map:get_field(sf.x + 511, sf.y + 502), false, true)
   connected_road(player, building.flag, "r,r,r")
   connected_road(player, building.flag, "br,br|br,br|br,bl|br,br|bl,br")

   -- (near trees)
   building = player:place_building("barbarians_lumberjacks_hut", map:get_field(sf.x + 2, sf.y + 502), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_rangers_hut", map:get_field(sf.x + 4, sf.y + 502), false, true)
   connected_road(player, building.flag, "tr,tr")

   building = player:place_building("barbarians_lumberjacks_hut", map:get_field(sf.x + 5, sf.y + 500), false, true)
   connected_road(player, building.flag, "tr,tl")

   building = player:place_building("barbarians_rangers_hut", map:get_field(sf.x + 5, sf.y + 498), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_lumberjacks_hut", map:get_field(sf.x + 7, sf.y + 498), false, true)
   connected_road(player, building.flag, "tr,r")

   building = player:place_building("barbarians_rangers_hut", map:get_field(sf.x + 9, sf.y + 497), false, true)
   connected_road(player, building.flag, "tr,tr,r")

   -- Building materials medium
   building = player:place_building("barbarians_wood_hardener", map:get_field(sf.x + 7, sf.y + 502), false, true)
   connected_road(player, building.flag, "l,l,l")
   connected_road(player, building.flag, "tr,tl|tl,tr")

   building = player:place_building("barbarians_wood_hardener", map:get_field(sf.x + 3, sf.y + 505), false, true)
   connected_road(player, building.flag, "tr,tr,tr")
   connected_road(player, building.flag, "l,bl")

   building = player:place_building("barbarians_lime_kiln", map:get_field(sf.x + 511, sf.y + 506), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_lime_kiln", map:get_field(sf.x + 508, sf.y + 505), false, true)
   connected_road(player, building.flag, "tr,r")

   building = player:place_building("barbarians_reed_yard", map:get_field(sf.x + 511, sf.y + 509), false, true)
   connected_road(player, building.flag, "tr,tr,tl")
   connected_road(player, building.flag, "bl,bl,br")

   building = player:place_building("barbarians_reed_yard", map:get_field(sf.x + 4, sf.y + 508), false, true)
   connected_road(player, building.flag, "tr,tr|tl,tr|tl,tl")
   connected_road(player, building.flag, "l,l,l")

   building = player:place_building("barbarians_charcoal_kiln", map:get_field(sf.x + 1, sf.y + 508), false, true)
   building = player:place_building("barbarians_charcoal_kiln", map:get_field(sf.x + 508, sf.y), false, true)

   -- Food and transport small
   building = player:place_building("barbarians_well", map:get_field(sf.x + 504, sf.y), false, true)
   building = player:place_building("barbarians_well", map:get_field(sf.x + 4, sf.y), false, true)
   building = player:place_building("barbarians_well", map:get_field(sf.x + 510, sf.y + 11), false, true)

   -- (near water)
   building = player:place_building("barbarians_fishers_hut", map:get_field(sf.x + 3, sf.y + 3), false, true)
   connected_road(player, building.flag, "tr,tl,tl")

   building = player:place_building("barbarians_fishers_hut", map:get_field(sf.x + 2, sf.y + 5), false, true)
   connected_road(player, building.flag, "l,l,tl")

   building = player:place_building("barbarians_fishers_hut", map:get_field(sf.x + 3, sf.y + 9), false, true)
   connected_road(player, building.flag, "bl,bl|bl,bl")

   building = player:place_building("barbarians_fishers_hut", map:get_field(sf.x + 5, sf.y + 12), false, true)
   connected_road(player, building.flag, "bl,bl,bl|bl,bl")

   -- (near animals)
   building = player:place_building("barbarians_hunters_hut", map:get_field(sf.x + 506, sf.y + 502), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_gamekeepers_hut", map:get_field(sf.x + 504, sf.y + 502), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_hunters_hut", map:get_field(sf.x + 502, sf.y + 502), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_gamekeepers_hut", map:get_field(sf.x + 500, sf.y + 502), false, true)
   connected_road(player, building.flag, "r,r")

   -- Food and transport medium
   building = player:place_building("barbarians_bakery", map:get_field(sf.x + 510, sf.y + 14), false, true)
   connected_road(player, building.flag, "tr,tl,tl")

   building = player:place_building("barbarians_bakery", map:get_field(sf.x + 0, sf.y + 18), false, true)
   connected_road(player, building.flag, "tr,tl,tl")

   building = player:place_building("barbarians_micro_brewery", map:get_field(sf.x + 3, sf.y + 26), false, true)
   connected_road(player, building.flag, "l,l")

   building = player:place_building("barbarians_micro_brewery", map:get_field(sf.x + 5, sf.y + 26), false, true)
   connected_road(player, building.flag, "l,l")

   building = player:place_building("barbarians_brewery", map:get_field(sf.x + 7, sf.y + 26), false, true)
   connected_road(player, building.flag, "l,l")

   building = player:place_building("barbarians_brewery", map:get_field(sf.x + 9, sf.y + 26), false, true)
   connected_road(player, building.flag, "l,l")

   building = player:place_building("barbarians_tavern", map:get_field(sf.x + 503, sf.y + 22), false, true)
   connected_road(player, building.flag, "r,tr,tr")

   building = player:place_building("barbarians_tavern", map:get_field(sf.x + 501, sf.y + 22), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_inn", map:get_field(sf.x + 499, sf.y + 22), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_inn", map:get_field(sf.x + 497, sf.y + 22), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_big_inn", map:get_field(sf.x + 495, sf.y + 22), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_big_inn", map:get_field(sf.x + 493, sf.y + 22), false, true)
   connected_road(player, building.flag, "r,r")


   -- Food and transport big
   building = player:place_building("barbarians_farm", map:get_field(sf.x + 505, sf.y + 20), false, true)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = player:place_building("barbarians_farm", map:get_field(sf.x + 507, sf.y + 16), false, true)
   connected_road(player, building.flag, "tr,tr|tl,tr")

   building = player:place_building("barbarians_farm", map:get_field(sf.x + 509, sf.y + 21), false, true)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = player:place_building("barbarians_farm", map:get_field(sf.x + 511, sf.y + 17), false, true)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = player:place_building("barbarians_farm", map:get_field(sf.x + 511, sf.y + 25), false, true)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = player:place_building("barbarians_farm", map:get_field(sf.x + 1, sf.y + 21), false, true)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = player:place_building("barbarians_farm", map:get_field(sf.x + 1, sf.y + 26), false, true)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = player:place_building("barbarians_farm", map:get_field(sf.x + 3, sf.y + 22), false, true)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = player:place_building("barbarians_cattlefarm", map:get_field(sf.x + 7, sf.y + 21), false, true)
   connected_road(player, building.flag, "l,l,tl")

   building = player:place_building("barbarians_cattlefarm", map:get_field(sf.x + 9, sf.y + 22), false, true)
   connected_road(player, building.flag, "l,l,tl")
   connected_road(player, building.flag, "tr,tr|r,tr")

   building = player:place_building("barbarians_weaving_mill", map:get_field(sf.x + 4, sf.y + 15), false, true)
   building = player:place_building("barbarians_weaving_mill", map:get_field(sf.x + 2, sf.y + 11), false, true)


   -- Tools, weapons and training sites medium
   building = player:place_building("barbarians_smelting_works", map:get_field(sf.x + 506, sf.y + 12), false, true)
   building = player:place_building("barbarians_smelting_works", map:get_field(sf.x + 508, sf.y + 12), false, true)

   building = player:place_building("barbarians_metal_workshop", map:get_field(sf.x + 496, sf.y + 18), false, true)
   connected_road(player, building.flag, "tr,tr")

   building = player:place_building("barbarians_metal_workshop", map:get_field(sf.x + 497, sf.y + 16), false, true)
   connected_road(player, building.flag, "tr,tr")

   building = player:place_building("barbarians_warmill", map:get_field(sf.x + 498, sf.y + 18), false, true)
   connected_road(player, building.flag, "tr,tr")

   building = player:place_building("barbarians_warmill", map:get_field(sf.x + 499, sf.y + 16), false, true)
   connected_road(player, building.flag, "tr,tr")

   building = player:place_building("barbarians_ax_workshop", map:get_field(sf.x + 500, sf.y + 18), false, true)
   connected_road(player, building.flag, "tr,tr")

   building = player:place_building("barbarians_ax_workshop", map:get_field(sf.x + 501, sf.y + 16), false, true)
   connected_road(player, building.flag, "tr,tr")

   building = player:place_building("barbarians_barracks", map:get_field(sf.x + 502, sf.y + 18), false, true)
   connected_road(player, building.flag, "tr,tr")

   building = player:place_building("barbarians_barracks", map:get_field(sf.x + 503, sf.y + 16), false, true)
   connected_road(player, building.flag, "tr,tr")


   -- Tools, weapons and training sites big
   building = player:place_building("barbarians_battlearena", map:get_field(sf.x + 489, sf.y + 4), false, true)
   building = player:place_building("barbarians_battlearena", map:get_field(sf.x + 488, sf.y + 9), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_trainingcamp", map:get_field(sf.x + 496, sf.y - 1), false, true)
   building = player:place_building("barbarians_trainingcamp", map:get_field(sf.x + 493, sf.y + 509), false, true)
   connected_road(player, building.flag, "br,br")

   building = player:place_building("barbarians_helmsmithy", map:get_field(sf.x + 502, sf.y + 509), false, true)
   building = player:place_building("barbarians_helmsmithy", map:get_field(sf.x + 502, sf.y + 506), false, true)
   connected_road(player, building.flag, "br,bl,bl")

   -- Mines
   building = player:place_building("barbarians_granitemine", map:get_field(sf.x + 504, sf.y + 4), false, true)
   connected_road(player, building.flag, "tr,tr|tr,tr")
   connected_road(player, building.flag, "br,r")

   building = player:place_building("barbarians_granitemine", map:get_field(sf.x + 506, sf.y + 5), false, true)
   connected_road(player, building.flag, "tr,tr,tr|tr,tr")

   building = player:place_building("barbarians_granitemine", map:get_field(sf.x + 505, sf.y + 8), false, true)
   connected_road(player, building.flag, "tr,tr,tl")

   building = player:place_building("barbarians_granitemine", map:get_field(sf.x + 503, sf.y + 8), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_coalmine", map:get_field(sf.x + 500, sf.y + 1), false, true)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = player:place_building("barbarians_coalmine", map:get_field(sf.x + 498, sf.y + 2), false, true)
   connected_road(player, building.flag, "tr,r")

   building = player:place_building("barbarians_coalmine_deep", map:get_field(sf.x + 497, sf.y + 3), false, true)
   connected_road(player, building.flag, "r,tr")

   building = player:place_building("barbarians_coalmine_deeper", map:get_field(sf.x + 502, sf.y + 3), false, true)
   connected_road(player, building.flag, "l,tl,tl")

   building = player:place_building("barbarians_ironmine", map:get_field(sf.x + 501, sf.y + 8), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_ironmine", map:get_field(sf.x + 499, sf.y + 8), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_ironmine_deep", map:get_field(sf.x + 497, sf.y + 8), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_ironmine_deeper", map:get_field(sf.x + 500, sf.y + 12), false, true)
   connected_road(player, building.flag, "tr,tr,tr,tl")
   connected_road(player, building.flag, "br,bl")

   building = player:place_building("barbarians_goldmine", map:get_field(sf.x + 495, sf.y + 8), false, true)
   connected_road(player, building.flag, "r,r")

   building = player:place_building("barbarians_goldmine", map:get_field(sf.x + 493, sf.y + 8), false, true)
   connected_road(player, building.flag, "r,r")
   connected_road(player, building.flag, "bl,bl|l,bl")

   building = player:place_building("barbarians_goldmine_deep", map:get_field(sf.x + 495, sf.y + 4), false, true)
   connected_road(player, building.flag, "r,tr")

   building = player:place_building("barbarians_goldmine_deeper", map:get_field(sf.x + 493, sf.y + 4), false, true)
   connected_road(player, building.flag, "r,r")
   connected_road(player, building.flag, "l,l,tl")

   -- Construction
   player:place_building("barbarians_helmsmithy", map:get_field(sf.x + 510, sf.y + 3), true, true)
end
