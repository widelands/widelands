-- This script places buildings of all types.
-- Split up into multiple functions to make debugging easier.
-- Main function is on the bottom.

local function init_militarysites(player, sf, total_previous_buildings)
   print("============================================")
   print("Military")
   print("--------------------------------------------")

   -- Get some territory
   place_initial_militarysites(map, sf, player, "barbarians_fortress")

   -- Military small
   place_safe_building(player, "barbarians_scouts_hut", sf, 21, 16)
   place_safe_building(player, "barbarians_scouts_hut", sf, 13, 18)

   place_militarysite(player, "barbarians_sentry", sf, 25, 8)
   place_militarysite(player, "barbarians_sentry", sf, 24, -1)
   place_militarysite(player, "barbarians_sentry", sf, 19, -12)
   place_militarysite(player, "barbarians_sentry", sf, 16, -13)
   place_militarysite(player, "barbarians_sentry", sf, 15, -17)
   place_militarysite(player, "barbarians_sentry", sf, 24, -11)

   -- Military medium
   place_militarysite(player, "barbarians_barrier", sf, 9, -13)
   place_militarysite(player, "barbarians_barrier", sf, 6, 28)
   place_militarysite(player, "barbarians_barrier", sf, -3, 23)

   place_militarysite(player, "barbarians_tower", sf, -7, -18)
   place_militarysite(player, "barbarians_tower", sf, -21, 23)
   place_militarysite(player, "barbarians_tower", sf, 23, 12)

   -- Military big
   place_militarysite(player, "barbarians_citadel", sf, -9, -12)
   place_militarysite(player, "barbarians_citadel", sf, 11, 28)
   place_militarysite(player, "barbarians_citadel", sf, -5, 26)

   return count_buildings(player, total_previous_buildings, 29)
end


local function init_warehouses(player, sf, total_previous_buildings)
   print("============================================")
   print("Warehouses")
   print("--------------------------------------------")

   -- Ports and warehouses
   place_warehouse(player, "barbarians_port", sf, 11, -3)
   building = place_warehouse(player, "barbarians_port", sf, 18, 13)
   connected_road("normal", player, building.flag, "br,bl,bl|br,bl")

   building = place_warehouse(player, "barbarians_warehouse", sf, -14, -11)
   connected_road("normal", player, building.flag, "r,br,r")

   building = place_warehouse(player, "barbarians_warehouse", sf, 24, 10)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr|tl,tr")
   connected_road("normal", player, building.flag, "bl,bl|bl,bl|bl,bl|bl,bl")

   building = place_warehouse(player, "barbarians_headquarters_interim", sf, 11, -17)
   connected_road("normal", player, building.flag, "br,br,br|br,br|br,br|br,br")

   return count_buildings(player, total_previous_buildings, 5)
end

local function init_shipconstruction(player, sf, total_previous_buildings)
   print("============================================")
   print("Ships and ferries")
   print("--------------------------------------------")

   -- Shipyards and ferry yards
   building = place_safe_building(player, "barbarians_shipyard", sf, 15, 18)
   building = place_safe_building(player, "barbarians_shipyard", sf, 8, -2)
   connected_road("normal", player, building.flag, "bl,br")

   building = place_safe_building(player, "barbarians_ferry_yard", sf, 8, 13)
   connected_road("normal", player, building.flag, "bl,bl,bl|bl,bl")

   building = place_safe_building(player, "barbarians_ferry_yard", sf, 18, -1)
   connected_road("normal", player, building.flag, "tr,r|tr,r")

   -- Waterway
   connected_road("normal", player, building.flag, "bl,bl")

   local flag = get_safe_field(sf, 17, 2).immovable
   assert_not_nil(flag)
   connected_road("waterway", player, flag, "bl,br,br,br,br,br,br,br")
   flag = get_safe_field(sf, 20, 10).immovable
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
   building = place_safe_building(player, "barbarians_quarry", sf, -4, -10)
   connected_road("normal", player, building.flag, "r,r,r")
   connected_road("normal", player, building.flag, "br,br|br,br,r")

   building = place_safe_building(player, "barbarians_quarry", sf, -1, -10)
   connected_road("normal", player, building.flag, "r,r,r")
   connected_road("normal", player, building.flag, "br,br|br,br|br,bl|br,br|bl,br")

   -- (near trees)
   building = place_safe_building(player, "barbarians_lumberjacks_hut", sf, 2, -10)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_rangers_hut", sf, 4, -10)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_lumberjacks_hut", sf, 5, -12)
   connected_road("normal", player, building.flag, "tr,tl")

   building = place_safe_building(player, "barbarians_rangers_hut", sf, 5, -14)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_lumberjacks_hut", sf, 7, -14)
   connected_road("normal", player, building.flag, "tr,r")

   building = place_safe_building(player, "barbarians_rangers_hut", sf, 9, -15)
   connected_road("normal", player, building.flag, "tr,tr,r")

   -- Building materials medium
   building = place_safe_building(player, "barbarians_wood_hardener", sf, 7, -10)
   connected_road("normal", player, building.flag, "l,l,l")
   connected_road("normal", player, building.flag, "tr,tl|tl,tr")

   building = place_safe_building(player, "barbarians_wood_hardener", sf, 3, -7)
   connected_road("normal", player, building.flag, "tr,tr,tr")
   connected_road("normal", player, building.flag, "l,bl")

   building = place_safe_building(player, "barbarians_lime_kiln", sf, -1, -6)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_lime_kiln", sf, -4, -7)
   connected_road("normal", player, building.flag, "tr,r")

   building = place_safe_building(player, "barbarians_reed_yard", sf, -1, -3)
   connected_road("normal", player, building.flag, "tr,tr,tl")
   connected_road("normal", player, building.flag, "bl,bl,br")

   building = place_safe_building(player, "barbarians_reed_yard", sf, 4, -4)
   connected_road("normal", player, building.flag, "tr,tr|tl,tr|tl,tl")
   connected_road("normal", player, building.flag, "l,l,l")

   building = place_safe_building(player, "barbarians_charcoal_kiln", sf, 1, -4)
   building = place_safe_building(player, "barbarians_charcoal_kiln", sf, -4, 0)

   return count_buildings(player, total_previous_buildings, 16)
end

local function init_food_and_transport(player, sf, total_previous_buildings)
   print("============================================")
   print("Food and transport")
   print("--------------------------------------------")

   -- Food and transport small
   building = place_safe_building(player, "barbarians_well", sf, -8, 0)
   building = place_safe_building(player, "barbarians_well", sf, 4, 0)
   building = place_safe_building(player, "barbarians_well", sf, -2, 11)

   -- (near water)
   building = place_safe_building(player, "barbarians_fishers_hut", sf, 3, 3)
   connected_road("normal", player, building.flag, "tr,tl,tl")

   building = place_safe_building(player, "barbarians_fishers_hut", sf, 2, 5)
   connected_road("normal", player, building.flag, "l,l,tl")

   building = place_safe_building(player, "barbarians_fishers_hut", sf, 3, 9)
   connected_road("normal", player, building.flag, "bl,bl|bl,bl")

   building = place_safe_building(player, "barbarians_fishers_hut", sf, 5, 12)
   connected_road("normal", player, building.flag, "bl,bl,bl|bl,bl")

   -- (near animals)
   building = place_safe_building(player, "barbarians_hunters_hut", sf, -6, -10)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_gamekeepers_hut", sf, -8, -10)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_hunters_hut", sf, -10, -10)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_gamekeepers_hut", sf, -12, -10)
   connected_road("normal", player, building.flag, "r,r")

   -- Food and transport medium
   building = place_safe_building(player, "barbarians_bakery", sf, -2, 14)
   connected_road("normal", player, building.flag, "tr,tl,tl")

   building = place_safe_building(player, "barbarians_bakery", sf, 0, 18)
   connected_road("normal", player, building.flag, "tr,tl,tl")

   building = place_safe_building(player, "barbarians_micro_brewery", sf, 3, 26)
   connected_road("normal", player, building.flag, "l,l")

   building = place_safe_building(player, "barbarians_micro_brewery", sf, 5, 26)
   connected_road("normal", player, building.flag, "l,l")

   building = place_safe_building(player, "barbarians_brewery", sf, 7, 26)
   connected_road("normal", player, building.flag, "l,l")

   building = place_safe_building(player, "barbarians_brewery", sf, 9, 26)
   connected_road("normal", player, building.flag, "l,l")

   building = place_safe_building(player, "barbarians_tavern", sf, -9, 22)
   connected_road("normal", player, building.flag, "r,tr,tr")

   building = place_safe_building(player, "barbarians_tavern", sf, -11, 22)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_inn", sf, -13, 22)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_inn", sf, -15, 22)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_big_inn", sf, -17, 22)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_big_inn", sf, -19, 22)
   connected_road("normal", player, building.flag, "r,r")


   -- Food and transport big
   building = place_safe_building(player, "barbarians_farm", sf, -7, 20)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_farm", sf, -5, 16)
   connected_road("normal", player, building.flag, "tr,tr|tl,tr")

   building = place_safe_building(player, "barbarians_farm", sf, -3, 21)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_farm", sf, -1, 17)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_farm", sf, -1, 25)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_farm", sf, 1, 21)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_farm", sf, 1, 26)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_farm", sf, 3, 22)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_cattlefarm", sf, 7, 21)
   connected_road("normal", player, building.flag, "l,l,tl")

   building = place_safe_building(player, "barbarians_cattlefarm", sf, 9, 22)
   connected_road("normal", player, building.flag, "l,l,tl")
   connected_road("normal", player, building.flag, "tr,tr|r,tr")

   building = place_safe_building(player, "barbarians_weaving_mill", sf, 4, 15)
   building = place_safe_building(player, "barbarians_weaving_mill", sf, 2, 11)

   return count_buildings(player, total_previous_buildings, 35)
end

local function init_tools_and_training(player, sf, total_previous_buildings)
   print("============================================")
   print("Tools and training")
   print("--------------------------------------------")

   -- Tools, weapons and training sites medium
   building = place_safe_building(player, "barbarians_smelting_works", sf, -6, 12)
   building = place_safe_building(player, "barbarians_smelting_works", sf, -4, 12)

   building = place_safe_building(player, "barbarians_metal_workshop", sf, -16, 18)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_metal_workshop", sf, -15, 16)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_warmill", sf, -14, 18)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_warmill", sf, -13, 16)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_ax_workshop", sf, -12, 18)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_ax_workshop", sf, -11, 16)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_barracks", sf, -10, 18)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "barbarians_barracks", sf, -9, 16)
   connected_road("normal", player, building.flag, "tr,tr")


   -- Tools, weapons and training sites big
   building = place_safe_building(player, "barbarians_battlearena", sf, -17, -5)
   connected_road("normal", player, building.flag, "bl,bl,l")
   connected_road("normal", player, building.flag, "tr,tr,tl,tr")

   building = place_safe_building(player, "barbarians_battlearena", sf, -16, -9)
   connected_road("normal", player, building.flag, "tr,r,tr")

   building = place_safe_building(player, "barbarians_trainingcamp", sf, -16, -1)
   connected_road("normal", player, building.flag, "tr,r|tr,r")

   building = place_safe_building(player, "barbarians_trainingcamp", sf, -19, -3)
   connected_road("normal", player, building.flag, "br,br|r,r")

   building = place_safe_building(player, "barbarians_helmsmithy", sf, -10, -3)
   building = place_safe_building(player, "barbarians_helmsmithy", sf, -10, -6)
   connected_road("normal", player, building.flag, "br,bl,bl")

   return count_buildings(player, total_previous_buildings, 16)
end

local function init_mines(player, sf, total_previous_buildings)
   print("============================================")
   print("Mines")
   print("--------------------------------------------")

   -- Mines
   building = place_safe_building(player, "barbarians_granitemine", sf, -8, 4)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")
   connected_road("normal", player, building.flag, "br,r")

   building = place_safe_building(player, "barbarians_granitemine", sf, -6, 5)
   connected_road("normal", player, building.flag, "tr,tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_granitemine", sf, -7, 8)
   connected_road("normal", player, building.flag, "tr,tr,tl")

   building = place_safe_building(player, "barbarians_granitemine", sf, -9, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_coalmine", sf, -12, 1)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "barbarians_coalmine", sf, -14, 2)
   connected_road("normal", player, building.flag, "tr,r")

   building = place_safe_building(player, "barbarians_coalmine_deep", sf, -15, 3)
   connected_road("normal", player, building.flag, "r,tr")

   building = place_safe_building(player, "barbarians_coalmine_deeper", sf, -10, 3)
   connected_road("normal", player, building.flag, "l,tl,tl")

   building = place_safe_building(player, "barbarians_ironmine", sf, -11, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_ironmine", sf, -13, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_ironmine_deep", sf, -15, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_ironmine_deeper", sf, -12, 12)
   connected_road("normal", player, building.flag, "tr,tr,tr,tl")
   connected_road("normal", player, building.flag, "br,bl")

   building = place_safe_building(player, "barbarians_goldmine", sf, -17, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_goldmine", sf, -19, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "barbarians_goldmine_deep", sf, -17, 4)
   connected_road("normal", player, building.flag, "r,tr")

   building = place_safe_building(player, "barbarians_goldmine_deeper", sf, -19, 4)
   connected_road("normal", player, building.flag, "r,r")
   connected_road("normal", player, building.flag, "l,l,tl")

   return count_buildings(player, total_previous_buildings, 16)
end


-- Place buildings for the Barbarian tribe
function init_barbarians(player)
   assert_equal("barbarians", player.tribe.name)

   local sf = wl.Game().map.player_slots[player.number].starting_field

   -- Headquarters
   place_warehouse(player, "barbarians_headquarters", sf, 0, 0)

   local total_buildings = init_militarysites(player, sf, 1)
   total_buildings = init_warehouses(player, sf, total_buildings)
   total_buildings = init_shipconstruction(player, sf, total_buildings)
   total_buildings = init_buildingmaterials(player, sf, total_buildings)
   total_buildings = init_food_and_transport(player, sf, total_buildings)
   total_buildings = init_tools_and_training(player, sf, total_buildings)
   total_buildings = init_mines(player, sf, total_buildings)

   -- Construction
   player:place_building("barbarians_helmsmithy", get_safe_field(sf, -2, 3), true, true)

   -- Test ships
   place_player_ship(player.number)
end


-- Run the tests
run(function()
   local playernumber = 1
   local expected_number_of_buildings = 122

   print("Placing buildings for Player 1")
   init_barbarians(game.players[playernumber])

   -- Verify that nothing went wrong with placing the buildings,
   -- and that a building of each type has been placed.
   verify_buildings(playernumber, expected_number_of_buildings)

   -- Sleep a bit so the tribe units can run some programs
   sleep_with_fps(30 * 10)

   -- Test saveloading and verify the buildings again
   stable_save(game, "barbarian_tribe", 1000)

   -- Did buildings saveload OK?
   verify_buildings(playernumber, expected_number_of_buildings)

   check_reload_version()
end)
