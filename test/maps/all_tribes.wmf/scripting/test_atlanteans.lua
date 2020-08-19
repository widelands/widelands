-- This script places buildings of all types.
-- Split up into multiple functions to make debugging easier.
-- Main function is on the bottom.

local function init_militarysites(player, sf, total_previous_buildings)
   print("============================================")
   print("Military")
   print("--------------------------------------------")

   -- Get some territory
   place_initial_militarysites(map, sf, player, "atlanteans_castle")

   -- Military small
   place_safe_building(player, "atlanteans_scouts_house", sf, 21, 16)
   place_safe_building(player, "atlanteans_scouts_house", sf, 13, 18)

   place_militarysite(player, "atlanteans_guardhouse", sf, 25, 8)
   place_militarysite(player, "atlanteans_guardhouse", sf, 24, -1)
   place_militarysite(player, "atlanteans_guardhouse", sf, 19, 500)

   place_militarysite(player, "atlanteans_tower_small", sf, 16, 499)
   place_militarysite(player, "atlanteans_tower_small", sf, 15, 495)
   place_militarysite(player, "atlanteans_tower_small", sf, 24, 501)

   -- Military medium
   place_militarysite(player, "atlanteans_guardhall", sf, 9, 499)
   place_militarysite(player, "atlanteans_guardhall", sf, 6, 28)

   place_militarysite(player, "atlanteans_tower", sf, 509, 23)
   place_militarysite(player, "atlanteans_tower", sf, 505, 494)

   place_militarysite(player, "atlanteans_tower_high", sf, 491, 23)
   place_militarysite(player, "atlanteans_tower_high", sf, 23, 12)

   -- Military big
   place_militarysite(player, "atlanteans_castle", sf, 503, 500)
   place_militarysite(player, "atlanteans_castle", sf, 11, 28)
   place_militarysite(player, "atlanteans_castle", sf, 507, 26)

   return count_buildings(player, total_previous_buildings, 29)
end


local function init_warehouses(player, sf, total_previous_buildings)
   print("============================================")
   print("Warehouses")
   print("--------------------------------------------")

   -- Ports and warehouses
   place_warehouse(player, "atlanteans_port", sf, 11, 509)
   building = place_warehouse(player, "atlanteans_port", sf, 18, 13)
   connected_road("normal", player, building.flag, "br,bl,bl|br,bl")

   building = place_warehouse(player, "atlanteans_warehouse", sf, 498, 501)
   connected_road("normal", player, building.flag, "r,br,r")

   building = place_warehouse(player, "atlanteans_warehouse", sf, 24, 10)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr|tl,tr")
   connected_road("normal", player, building.flag, "bl,bl|bl,bl|bl,bl|bl,bl")

   building = place_warehouse(player, "atlanteans_warehouse", sf, 11, 495)
   connected_road("normal", player, building.flag, "br,br,br|br,br|br,br|br,br")

   return count_buildings(player, total_previous_buildings, 5)
end

local function init_shipconstruction(player, sf, total_previous_buildings)
   print("============================================")
   print("Ships and ferries")
   print("--------------------------------------------")

   -- Shipyards and ferry yards
   building = place_safe_building(player, "atlanteans_shipyard", sf, 15, 18)
   building = place_safe_building(player, "atlanteans_shipyard", sf, 8, 510)
   connected_road("normal", player, building.flag, "bl,br")

   building = place_safe_building(player, "atlanteans_ferry_yard", sf, 8, 13)
   connected_road("normal", player, building.flag, "bl,bl,bl|bl,bl")

   building = place_safe_building(player, "atlanteans_ferry_yard", sf, 18, -1)
   connected_road("normal", player, building.flag, "tr,r|tr,r")

   -- Waterway
   connected_road("normal", player, building.flag, "bl,bl")

   local flag = get_safe_field(player, sf, 17, 2).immovable
   assert_not_nil(flag)
   connected_road("waterway", player, flag, "bl,br,br,br,br,br,br,br")
   flag = get_safe_field(player, sf, 20, 10).immovable
   assert_not_nil(flag)
   connected_road("busy", player, flag, "bl,bl,bl,bl")

   return count_buildings(player, total_previous_buildings, 4)
end

local function init_buildingmaterials(player, sf, total_previous_buildings)
   print("============================================")
   print("Buildingmaterials")
   print("--------------------------------------------")

   -- Building materials small
   -- (near rocks)
   building = place_safe_building(player, "atlanteans_quarry", sf, 508, 502)
   connected_road("normal", player, building.flag, "r,r,r")
   connected_road("normal", player, building.flag, "br,br|br,br,r")

   building = place_safe_building(player, "atlanteans_quarry", sf, 511, 502)
   connected_road("normal", player, building.flag, "r,r,r")
   connected_road("normal", player, building.flag, "br,br|br,br|br,bl|br,br|bl,br")

   -- (near trees)
   building = place_safe_building(player, "atlanteans_woodcutters_house", sf, 2, 502)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_foresters_house", sf, 4, 502)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "atlanteans_woodcutters_house", sf, 5, 500)
   connected_road("normal", player, building.flag, "tr,tl")

   building = place_safe_building(player, "atlanteans_foresters_house", sf, 5, 498)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_gold_spinning_mill", sf, 7, 498)
   connected_road("normal", player, building.flag, "tr,r")

   building = place_safe_building(player, "atlanteans_gold_spinning_mill", sf, 9, 497)
   connected_road("normal", player, building.flag, "tr,tr,r")

   -- Building materials medium
   building = place_safe_building(player, "atlanteans_sawmill", sf, 7, 502)
   connected_road("normal", player, building.flag, "l,l,l")
   connected_road("normal", player, building.flag, "tr,tl|tl,tr")

   building = place_safe_building(player, "atlanteans_sawmill", sf, 3, 505)
   connected_road("normal", player, building.flag, "tr,tr,tr")
   connected_road("normal", player, building.flag, "l,bl")

   building = place_safe_building(player, "atlanteans_sawmill", sf, 511, 506)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_sawmill", sf, 508, 505)
   connected_road("normal", player, building.flag, "tr,r")

   building = place_safe_building(player, "atlanteans_charcoal_kiln", sf, 511, 509)
   connected_road("normal", player, building.flag, "tr,tr,tl")
   connected_road("normal", player, building.flag, "bl,bl,br")

   building = place_safe_building(player, "atlanteans_charcoal_kiln", sf, 4, 508)
   connected_road("normal", player, building.flag, "tr,tr|tl,tr|tl,tl")
   connected_road("normal", player, building.flag, "l,l,l")

   building = place_safe_building(player, "atlanteans_charcoal_kiln", sf, 1, 508)
   building = place_safe_building(player, "atlanteans_charcoal_kiln", sf, 508, 0)

   return count_buildings(player, total_previous_buildings, 16)
end

local function init_food_and_transport(player, sf, total_previous_buildings)
   print("============================================")
   print("Food and transport")
   print("--------------------------------------------")

   -- Food and transport small
   building = place_safe_building(player, "atlanteans_well", sf, 504, 0)
   building = place_safe_building(player, "atlanteans_well", sf, 4, 0)
   building = place_safe_building(player, "atlanteans_well", sf, 510, 11)

   -- (near water)
   building = place_safe_building(player, "atlanteans_fishers_house", sf, 3, 3)
   connected_road("normal", player, building.flag, "tr,tl,tl")

   building = place_safe_building(player, "atlanteans_fishbreeders_house", sf, 2, 5)
   connected_road("normal", player, building.flag, "l,l,tl")

   building = place_safe_building(player, "atlanteans_fishers_house", sf, 3, 9)
   connected_road("normal", player, building.flag, "bl,bl|bl,bl")

   building = place_safe_building(player, "atlanteans_fishbreeders_house", sf, 5, 12)
   connected_road("normal", player, building.flag, "bl,bl,bl|bl,bl")

   -- (near animals)
   building = place_safe_building(player, "atlanteans_hunters_house", sf, 506, 502)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_hunters_house", sf, 504, 502)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_hunters_house", sf, 502, 502)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_hunters_house", sf, 500, 502)
   connected_road("normal", player, building.flag, "r,r")

   -- Food and transport medium
   building = place_safe_building(player, "atlanteans_bakery", sf, 510, 14)
   connected_road("normal", player, building.flag, "tr,tl,tl")

   building = place_safe_building(player, "atlanteans_bakery", sf, 0, 18)
   connected_road("normal", player, building.flag, "tr,tl,tl")

   building = place_safe_building(player, "atlanteans_smokery", sf, 3, 26)
   connected_road("normal", player, building.flag, "l,l")

   building = place_safe_building(player, "atlanteans_smokery", sf, 5, 26)
   connected_road("normal", player, building.flag, "l,l")

   building = place_safe_building(player, "atlanteans_mill", sf, 7, 26)
   connected_road("normal", player, building.flag, "l,l")

   building = place_safe_building(player, "atlanteans_mill", sf, 9, 26)
   connected_road("normal", player, building.flag, "l,l")

   building = place_safe_building(player, "atlanteans_bakery", sf, 503, 22)
   connected_road("normal", player, building.flag, "r,tr,tr")

   building = place_safe_building(player, "atlanteans_smokery", sf, 501, 22)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_mill", sf, 499, 22)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_bakery", sf, 497, 22)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_smokery", sf, 495, 22)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_mill", sf, 493, 22)
   connected_road("normal", player, building.flag, "r,r")


   -- Food and transport big
   building = place_safe_building(player, "atlanteans_farm", sf, 505, 20)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "atlanteans_farm", sf, 507, 16)
   connected_road("normal", player, building.flag, "tr,tr|tl,tr")

   building = place_safe_building(player, "atlanteans_farm", sf, 509, 21)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "atlanteans_farm", sf, 511, 17)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "atlanteans_blackroot_farm", sf, 511, 25)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "atlanteans_blackroot_farm", sf, 1, 21)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "atlanteans_spiderfarm", sf, 1, 26)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "atlanteans_spiderfarm", sf, 3, 22)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "atlanteans_horsefarm", sf, 7, 21)
   connected_road("normal", player, building.flag, "l,l,tl")

   building = place_safe_building(player, "atlanteans_horsefarm", sf, 9, 22)
   connected_road("normal", player, building.flag, "l,l,tl")
   connected_road("normal", player, building.flag, "tr,tr|r,tr")

   building = place_safe_building(player, "atlanteans_weaving_mill", sf, 4, 15)
   building = place_safe_building(player, "atlanteans_weaving_mill", sf, 2, 11)

   return count_buildings(player, total_previous_buildings, 35)
end

local function init_tools_and_training(player, sf, total_previous_buildings)
   print("============================================")
   print("Tools and training")
   print("--------------------------------------------")

   -- Tools, weapons and training sites medium
   building = place_safe_building(player, "atlanteans_smelting_works", sf, 506, 12)
   building = place_safe_building(player, "atlanteans_smelting_works", sf, 508, 12)

   building = place_safe_building(player, "atlanteans_toolsmithy", sf, 496, 18)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "atlanteans_toolsmithy", sf, 497, 16)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "atlanteans_weaponsmithy", sf, 498, 18)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "atlanteans_weaponsmithy", sf, 499, 16)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "atlanteans_armorsmithy", sf, 500, 18)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "atlanteans_armorsmithy", sf, 501, 16)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "atlanteans_barracks", sf, 502, 18)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "atlanteans_barracks", sf, 503, 16)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "atlanteans_dungeon", sf, 495, 507)
   connected_road("normal", player, building.flag, "bl,bl,l")
   connected_road("normal", player, building.flag, "tr,tr,tl,tr")

   building = place_safe_building(player, "atlanteans_dungeon", sf, 496, 503)
   connected_road("normal", player, building.flag, "tr,r,tr")

   building = place_safe_building(player, "atlanteans_dungeon", sf, 496, -1)
   connected_road("normal", player, building.flag, "tr,r|tr,r")

   -- Tools, weapons and training sites big
   building = place_safe_building(player, "atlanteans_labyrinth", sf, 493, 509)
   connected_road("normal", player, building.flag, "br,br|r,r")

   building = place_safe_building(player, "atlanteans_labyrinth", sf, 502, 509)
   building = place_safe_building(player, "atlanteans_labyrinth", sf, 502, 506)
   connected_road("normal", player, building.flag, "br,bl,bl")

   return count_buildings(player, total_previous_buildings, 16)
end

local function init_mines(player, sf, total_previous_buildings)
   print("============================================")
   print("Mines")
   print("--------------------------------------------")

   -- Mines
   building = place_safe_building(player, "atlanteans_crystalmine", sf, 504, 4)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")
   connected_road("normal", player, building.flag, "br,r")

   building = place_safe_building(player, "atlanteans_crystalmine", sf, 506, 5)
   connected_road("normal", player, building.flag, "tr,tr,tr|tr,tr")

   building = place_safe_building(player, "atlanteans_crystalmine", sf, 505, 8)
   connected_road("normal", player, building.flag, "tr,tr,tl")

   building = place_safe_building(player, "atlanteans_crystalmine", sf, 503, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_coalmine", sf, 500, 1)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "atlanteans_coalmine", sf, 498, 2)
   connected_road("normal", player, building.flag, "tr,r")

   building = place_safe_building(player, "atlanteans_coalmine", sf, 497, 3)
   connected_road("normal", player, building.flag, "r,tr")

   building = place_safe_building(player, "atlanteans_coalmine", sf, 502, 3)
   connected_road("normal", player, building.flag, "l,tl,tl")

   building = place_safe_building(player, "atlanteans_ironmine", sf, 501, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_ironmine", sf, 499, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_ironmine", sf, 497, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_ironmine", sf, 500, 12)
   connected_road("normal", player, building.flag, "tr,tr,tr,tl")
   connected_road("normal", player, building.flag, "br,bl")

   building = place_safe_building(player, "atlanteans_goldmine", sf, 495, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_goldmine", sf, 493, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "atlanteans_goldmine", sf, 495, 4)
   connected_road("normal", player, building.flag, "r,tr")

   building = place_safe_building(player, "atlanteans_goldmine", sf, 493, 4)
   connected_road("normal", player, building.flag, "r,r")
   connected_road("normal", player, building.flag, "l,l,tl")

   return count_buildings(player, total_previous_buildings, 16)
end


-- Place buildings for Atlantean tribe
function init_atlanteans(player)
   assert_equal("atlanteans", player.tribe.name)

   local sf = wl.Game().map.player_slots[player.number].starting_field
   map = wl.Game().map

   -- Headquarters
   place_warehouse(player, "atlanteans_headquarters", sf, 0, 0)

   local total_buildings = init_militarysites(player, sf, 1)
   total_buildings = init_warehouses(player, sf, total_buildings)
   total_buildings = init_shipconstruction(player, sf, total_buildings)
   total_buildings = init_buildingmaterials(player, sf, total_buildings)
   total_buildings = init_food_and_transport(player, sf, total_buildings)
   total_buildings = init_tools_and_training(player, sf, total_buildings)
   total_buildings = init_mines(player, sf, total_buildings)

   -- Construction
   player:place_building("atlanteans_labyrinth", map:get_field((sf.x + 510) % 512, (sf.y + 3) % 512), true, true)

   -- Test ships
   place_player_ship(player.number)
end

-- Run the tests
run(function()
   local playernumber = 3
   local expected_number_of_buildings = 122

   print("Placing buildings for Player 3")
   init_atlanteans(game.players[playernumber])

   -- Verify that nothing went wrong with placing the buildings,
   -- and that a building of each type has been placed.
   verify_buildings(playernumber, expected_number_of_buildings)

   -- Sleep a bit so the tribe units can run some programs
   sleep_with_fps(30 * 10)

   -- Test saveloading and verify the buildings again
   stable_save(game, "atlantean_tribe", 1000)

   -- Did buildings saveload OK?
   verify_buildings(playernumber, expected_number_of_buildings)

   print("# All Tests passed.")
   mapview:close()
end)
