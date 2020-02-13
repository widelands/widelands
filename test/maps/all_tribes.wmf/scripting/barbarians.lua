-- ===============
-- Initialization
-- ===============

function init_barbarians(player)


   local tribe = player.tribe
   -- NOCOM assert that tribe is barbarians
   local sf = wl.Game().map.player_slots[player.number].starting_field
   map = wl.Game().map

   -- Headquarters
   place_warehouse(player, "barbarians_headquarters", sf.x, sf.y)

   -- Get some territory
   place_initial_militarysites(map, sf, player, "barbarians_fortress")

   -- Military small
   place_safe_building(player, "barbarians_scouts_hut", sf.x + 21, sf.y + 16)
   place_safe_building(player, "barbarians_scouts_hut", sf.x + 13, sf.y + 18)

   place_safe_building(player, "barbarians_sentry", sf.x + 27, sf.y + 8)
   place_safe_building(player, "barbarians_sentry", sf.x + 31, sf.y + 3)
   place_safe_building(player, "barbarians_sentry", sf.x + 33, sf.y + 3)
   place_safe_building(player, "barbarians_sentry", sf.x + 35, sf.y + 3)
   place_safe_building(player, "barbarians_sentry", sf.x + 27, sf.y + 507)
   place_safe_building(player, "barbarians_sentry", sf.x + 24, sf.y + 501)

   -- Military medium
   place_safe_building(player, "barbarians_barrier", sf.x + 9, sf.y + 499)
   place_safe_building(player, "barbarians_barrier", sf.x + 6, sf.y + 28)
   place_safe_building(player, "barbarians_barrier", sf.x + 509, sf.y + 23)

   place_safe_building(player, "barbarians_tower", sf.x + 486, sf.y + 7)
   place_safe_building(player, "barbarians_tower", sf.x + 491, sf.y + 23)
   place_safe_building(player, "barbarians_tower", sf.x + 26, sf.y + 11)

   -- Military big
   place_safe_building(player, "barbarians_citadel", sf.x + 503, sf.y + 500)
   place_safe_building(player, "barbarians_citadel", sf.x + 11, sf.y + 28)
   place_safe_building(player, "barbarians_citadel", sf.x + 507, sf.y + 26)

   -- Ports, and warehouses
   place_warehouse(player, "barbarians_port", sf.x + 11, sf.y + 509)
   building = place_warehouse(player, "barbarians_port", sf.x + 18, sf.y + 13)
   connected_road(player, building.flag, "br,bl,bl|br,bl")

   building = place_warehouse(player, "barbarians_warehouse", sf.x + 488, sf.y + 6)
   connected_road(player, building.flag, "tr,tr|r,tr|tr,tr")
   connected_road(player, building.flag, "br,br,br|br,br|br,br|br,r|r,r")

   building = place_warehouse(player, "barbarians_warehouse", sf.x + 24, sf.y + 10)
   connected_road(player, building.flag, "tr,tr|tr,tr|tl,tr")
   connected_road(player, building.flag, "bl,bl|bl,bl|bl,bl|bl,bl")

   building = place_warehouse(player, "barbarians_headquarters_interim", sf.x + 11, sf.y + 495)
   connected_road(player, building.flag, "br,br,br|br,br|br,br|br,br")

   -- Shipyards and ferry yards
   building = place_safe_building(player, "barbarians_shipyard", sf.x + 14, sf.y + 16)
   connected_road(player, building.flag, "bl,bl")

   building = place_safe_building(player, "barbarians_shipyard", sf.x + 8, sf.y)

   building = place_safe_building(player, "barbarians_ferry_yard", sf.x + 8, sf.y + 12)
   connected_road(player, building.flag, "bl,bl|bl,bl|bl,bl")

   building = place_safe_building(player, "barbarians_ferry_yard", sf.x + 18, sf.y + 1)
   connected_road(player, building.flag, "tr,tr,tr|tr,r")

   -- Building materials small
   -- (near rocks)
   building = place_safe_building(player, "barbarians_quarry", sf.x + 508, sf.y + 502)
   connected_road(player, building.flag, "r,r,r")
   connected_road(player, building.flag, "br,br|br,br,r")

   building = place_safe_building(player, "barbarians_quarry", sf.x + 511, sf.y + 502)
   connected_road(player, building.flag, "r,r,r")
   connected_road(player, building.flag, "br,br|br,br|br,bl|br,br|bl,br")

   -- (near trees)
   building = place_safe_building(player, "barbarians_lumberjacks_hut", sf.x + 2, sf.y + 502)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_rangers_hut", sf.x + 4, sf.y + 502)
   connected_road(player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_lumberjacks_hut", sf.x + 5, sf.y + 500)
   connected_road(player, building.flag, "tr,tl")

   building = place_safe_building(player, "barbarians_rangers_hut", sf.x + 5, sf.y + 498)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_lumberjacks_hut", sf.x + 7, sf.y + 498)
   connected_road(player, building.flag, "tr,r")

   building = place_safe_building(player, "barbarians_rangers_hut", sf.x + 9, sf.y + 497)
   connected_road(player, building.flag, "tr,tr,r")

   -- Building materials medium
   building = place_safe_building(player, "barbarians_wood_hardener", sf.x + 7, sf.y + 502)
   connected_road(player, building.flag, "l,l,l")
   connected_road(player, building.flag, "tr,tl|tl,tr")

   building = place_safe_building(player, "barbarians_wood_hardener", sf.x + 3, sf.y + 505)
   connected_road(player, building.flag, "tr,tr,tr")
   connected_road(player, building.flag, "l,bl")

   building = place_safe_building(player, "barbarians_lime_kiln", sf.x + 511, sf.y + 506)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_lime_kiln", sf.x + 508, sf.y + 505)
   connected_road(player, building.flag, "tr,r")

   building = place_safe_building(player, "barbarians_reed_yard", sf.x + 511, sf.y + 509)
   connected_road(player, building.flag, "tr,tr,tl")
   connected_road(player, building.flag, "bl,bl,br")

   building = place_safe_building(player, "barbarians_reed_yard", sf.x + 4, sf.y + 508)
   connected_road(player, building.flag, "tr,tr|tl,tr|tl,tl")
   connected_road(player, building.flag, "l,l,l")

   building = place_safe_building(player, "barbarians_charcoal_kiln", sf.x + 1, sf.y + 508)
   building = place_safe_building(player, "barbarians_charcoal_kiln", sf.x + 508, sf.y)

   -- Food and transport small
   building = place_safe_building(player, "barbarians_well", sf.x + 504, sf.y)
   building = place_safe_building(player, "barbarians_well", sf.x + 4, sf.y)
   building = place_safe_building(player, "barbarians_well", sf.x + 510, sf.y + 11)

   -- (near water)
   building = place_safe_building(player, "barbarians_fishers_hut", sf.x + 3, sf.y + 3)
   connected_road(player, building.flag, "tr,tl,tl")

   building = place_safe_building(player, "barbarians_fishers_hut", sf.x + 2, sf.y + 5)
   connected_road(player, building.flag, "l,l,tl")

   building = place_safe_building(player, "barbarians_fishers_hut", sf.x + 3, sf.y + 9)
   connected_road(player, building.flag, "bl,bl|bl,bl")

   building = place_safe_building(player, "barbarians_fishers_hut", sf.x + 5, sf.y + 12)
   connected_road(player, building.flag, "bl,bl,bl|bl,bl")

   -- (near animals)
   building = place_safe_building(player, "barbarians_hunters_hut", sf.x + 506, sf.y + 502)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_gamekeepers_hut", sf.x + 504, sf.y + 502)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_hunters_hut", sf.x + 502, sf.y + 502)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_gamekeepers_hut", sf.x + 500, sf.y + 502)
   connected_road(player, building.flag, "r,r")

   -- Food and transport medium
   building = place_safe_building(player, "barbarians_bakery", sf.x + 510, sf.y + 14)
   connected_road(player, building.flag, "tr,tl,tl")

   building = place_safe_building(player, "barbarians_bakery", sf.x + 0, sf.y + 18)
   connected_road(player, building.flag, "tr,tl,tl")

   building = place_safe_building(player, "barbarians_micro_brewery", sf.x + 3, sf.y + 26)
   connected_road(player, building.flag, "l,l")

   building = place_safe_building(player, "barbarians_micro_brewery", sf.x + 5, sf.y + 26)
   connected_road(player, building.flag, "l,l")

   building = place_safe_building(player, "barbarians_brewery", sf.x + 7, sf.y + 26)
   connected_road(player, building.flag, "l,l")

   building = place_safe_building(player, "barbarians_brewery", sf.x + 9, sf.y + 26)
   connected_road(player, building.flag, "l,l")

   building = place_safe_building(player, "barbarians_tavern", sf.x + 503, sf.y + 22)
   connected_road(player, building.flag, "r,tr,tr")

   building = place_safe_building(player, "barbarians_tavern", sf.x + 501, sf.y + 22)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_inn", sf.x + 499, sf.y + 22)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_inn", sf.x + 497, sf.y + 22)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_big_inn", sf.x + 495, sf.y + 22)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_big_inn", sf.x + 493, sf.y + 22)
   connected_road(player, building.flag, "r,r")


   -- Food and transport big
   building = place_safe_building(player, "barbarians_farm", sf.x + 505, sf.y + 20)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_farm", sf.x + 507, sf.y + 16)
   connected_road(player, building.flag, "tr,tr|tl,tr")

   building = place_safe_building(player, "barbarians_farm", sf.x + 509, sf.y + 21)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_farm", sf.x + 511, sf.y + 17)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_farm", sf.x + 511, sf.y + 25)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_farm", sf.x + 1, sf.y + 21)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_farm", sf.x + 1, sf.y + 26)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_farm", sf.x + 3, sf.y + 22)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_cattlefarm", sf.x + 7, sf.y + 21)
   connected_road(player, building.flag, "l,l,tl")

   building = place_safe_building(player, "barbarians_cattlefarm", sf.x + 9, sf.y + 22)
   connected_road(player, building.flag, "l,l,tl")
   connected_road(player, building.flag, "tr,tr|r,tr")

   building = place_safe_building(player, "barbarians_weaving_mill", sf.x + 4, sf.y + 15)
   building = place_safe_building(player, "barbarians_weaving_mill", sf.x + 2, sf.y + 11)


   -- Tools, weapons and training sites medium
   building = place_safe_building(player, "barbarians_smelting_works", sf.x + 506, sf.y + 12)
   building = place_safe_building(player, "barbarians_smelting_works", sf.x + 508, sf.y + 12)

   building = place_safe_building(player, "barbarians_metal_workshop", sf.x + 496, sf.y + 18)
   connected_road(player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_metal_workshop", sf.x + 497, sf.y + 16)
   connected_road(player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_warmill", sf.x + 498, sf.y + 18)
   connected_road(player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_warmill", sf.x + 499, sf.y + 16)
   connected_road(player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_ax_workshop", sf.x + 500, sf.y + 18)
   connected_road(player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_ax_workshop", sf.x + 501, sf.y + 16)
   connected_road(player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_barracks", sf.x + 502, sf.y + 18)
   connected_road(player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_barracks", sf.x + 503, sf.y + 16)
   connected_road(player, building.flag, "tr,tr")


   -- Tools, weapons and training sites big
   building = place_safe_building(player, "barbarians_battlearena", sf.x + 489, sf.y + 4)
   building = place_safe_building(player, "barbarians_battlearena", sf.x + 488, sf.y + 9)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_trainingcamp", sf.x + 496, sf.y - 1)
   building = place_safe_building(player, "barbarians_trainingcamp", sf.x + 493, sf.y + 509)
   connected_road(player, building.flag, "br,br")

   building = place_safe_building(player, "barbarians_helmsmithy", sf.x + 502, sf.y + 509)
   building = place_safe_building(player, "barbarians_helmsmithy", sf.x + 502, sf.y + 506)
   connected_road(player, building.flag, "br,bl,bl")

   -- Mines
   building = place_safe_building(player, "barbarians_granitemine", sf.x + 504, sf.y + 4)
   connected_road(player, building.flag, "tr,tr|tr,tr")
   connected_road(player, building.flag, "br,r")

   building = place_safe_building(player, "barbarians_granitemine", sf.x + 506, sf.y + 5)
   connected_road(player, building.flag, "tr,tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_granitemine", sf.x + 505, sf.y + 8)
   connected_road(player, building.flag, "tr,tr,tl")

   building = place_safe_building(player, "barbarians_granitemine", sf.x + 503, sf.y + 8)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_coalmine", sf.x + 500, sf.y + 1)
   connected_road(player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_coalmine", sf.x + 498, sf.y + 2)
   connected_road(player, building.flag, "tr,r")

   building = place_safe_building(player, "barbarians_coalmine_deep", sf.x + 497, sf.y + 3)
   connected_road(player, building.flag, "r,tr")

   building = place_safe_building(player, "barbarians_coalmine_deeper", sf.x + 502, sf.y + 3)
   connected_road(player, building.flag, "l,tl,tl")

   building = place_safe_building(player, "barbarians_ironmine", sf.x + 501, sf.y + 8)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_ironmine", sf.x + 499, sf.y + 8)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_ironmine_deep", sf.x + 497, sf.y + 8)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_ironmine_deeper", sf.x + 500, sf.y + 12)
   connected_road(player, building.flag, "tr,tr,tr,tl")
   connected_road(player, building.flag, "br,bl")

   building = place_safe_building(player, "barbarians_goldmine", sf.x + 495, sf.y + 8)
   connected_road(player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_goldmine", sf.x + 493, sf.y + 8)
   connected_road(player, building.flag, "r,r")
   connected_road(player, building.flag, "bl,bl|l,bl")

   building = place_safe_building(player, "barbarians_goldmine_deep", sf.x + 495, sf.y + 4)
   connected_road(player, building.flag, "r,tr")

   building = place_safe_building(player, "barbarians_goldmine_deeper", sf.x + 493, sf.y + 4)
   connected_road(player, building.flag, "r,r")
   connected_road(player, building.flag, "l,l,tl")

   -- Construction
   player:place_building("barbarians_helmsmithy", map:get_field((sf.x + 510) % 512, (sf.y + 3) % 512), true, true)
end
