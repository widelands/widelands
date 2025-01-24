-- This script places buildings of all types.
-- Split up into multiple functions to make debugging easier.
-- Main function is on the bottom.

local function init_militarysites(player, sf, total_previous_buildings)
   print("============================================")
   print("Military")
   print("--------------------------------------------")

   -- Get some territory
   place_initial_militarysites(map, sf, player, "frisians_fortress")

   -- Military small
   place_safe_building(player, "frisians_scouts_house", sf, 21, 16)
   place_safe_building(player, "frisians_scouts_house", sf, 13, 18)

   place_militarysite(player, "frisians_wooden_tower", sf, 25, 8)
   place_militarysite(player, "frisians_wooden_tower", sf, 24, -1)

   place_militarysite(player, "frisians_wooden_tower_high", sf, 19, -12)
   place_militarysite(player, "frisians_wooden_tower_high", sf, 16, -13)

   place_militarysite(player, "frisians_sentinel", sf, 15, -17)
   place_militarysite(player, "frisians_sentinel", sf, 24, -11)

   -- Military medium
   place_militarysite(player, "frisians_outpost", sf, 9, -13)
   place_militarysite(player, "frisians_outpost", sf, 6, 28)
   place_militarysite(player, "frisians_outpost", sf, -3, 23)

   place_militarysite(player, "frisians_outpost", sf, -7, -18)
   place_militarysite(player, "frisians_outpost", sf, -21, 23)
   place_militarysite(player, "frisians_outpost", sf, 23, 12)

   -- Military big
   place_militarysite(player, "frisians_tower", sf, -9, -12)
   place_militarysite(player, "frisians_tower", sf, 11, 28)
   place_militarysite(player, "frisians_tower", sf, -5, 26)

   return count_buildings(player, total_previous_buildings, 29)
end


local function init_warehouses(player, sf, total_previous_buildings)
   print("============================================")
   print("Warehouses")
   print("--------------------------------------------")

   -- Ports and warehouses
   place_warehouse(player, "frisians_port", sf, 11, -3)
   building = place_warehouse(player, "frisians_port", sf, 18, 13)
   connected_road("normal", player, building.flag, "br,bl,bl|br,bl")

   building = place_warehouse(player, "frisians_warehouse", sf, -14, -11)
   connected_road("normal", player, building.flag, "r,br,r")

   building = place_warehouse(player, "frisians_warehouse", sf, 24, 10)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr|tl,tr")
   connected_road("normal", player, building.flag, "bl,bl|bl,bl|bl,bl|bl,bl")

   building = place_warehouse(player, "frisians_warehouse", sf, 11, -17)
   connected_road("normal", player, building.flag, "br,br,br|br,br|br,br|br,br")

   return count_buildings(player, total_previous_buildings, 5)
end

local function init_markets(player, sf, total_previous_buildings)
   print("============================================")
   print("Markets")
   print("--------------------------------------------")

   -- Market
   building = place_market(player, "frisians_market", sf, 26, 59)
   connected_road("normal", player, building.flag, "bl,bl")

   return count_buildings(player, total_previous_buildings, 1)
end

local function init_shipconstruction(player, sf, total_previous_buildings)
   print("============================================")
   print("Ships and ferries")
   print("--------------------------------------------")

   -- Shipyards and ferry yards
   building = place_safe_building(player, "frisians_shipyard", sf, 15, 18)
   building = place_safe_building(player, "frisians_shipyard", sf, 8, -2)
   connected_road("normal", player, building.flag, "bl,br")

   building = place_safe_building(player, "frisians_ferry_yard", sf, 8, 13)
   connected_road("normal", player, building.flag, "bl,bl,bl|bl,bl")

   building = place_safe_building(player, "frisians_ferry_yard", sf, 18, -1)
   connected_road("normal", player, building.flag, "tr,r|tr,r")

   -- Waterway
   connected_road("normal", player, building.flag, "bl,bl")

   local flag = get_safe_field(sf, 17, 2).immovable
   assert_not_nil(flag)
   connected_road("waterway", player, flag, "bl,br,br,br,br,br,br,br")
   flag = get_safe_field(sf, 20, 10).immovable
   assert_not_nil(flag)
   connected_road("busy", player, flag, "bl,bl,bl,bl")

   -- Diking
   building = place_safe_building(player, "frisians_dikers_house", sf, 20, 1)
   connected_road("normal", player, building.flag, "l,l,l")

   return count_buildings(player, total_previous_buildings, 5)
end

local function init_buildingmaterials(player, sf, total_previous_buildings)
   print("============================================")
   print("Buildingmaterials")
   print("--------------------------------------------")

   -- Building materials small
   -- (near rocks)
   building = place_safe_building(player, "frisians_quarry", sf, -4, -10)
   connected_road("normal", player, building.flag, "r,r,r")
   connected_road("normal", player, building.flag, "br,br|br,br,r")

   building = place_safe_building(player, "frisians_quarry", sf, -1, -10)
   connected_road("normal", player, building.flag, "r,r,r")
   connected_road("normal", player, building.flag, "br,br|br,br|br,bl|br,br|bl,br")

   -- (near trees)
   building = place_safe_building(player, "frisians_woodcutters_house", sf, 2, -10)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_foresters_house", sf, 4, -10)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "frisians_woodcutters_house", sf, 5, -12)
   connected_road("normal", player, building.flag, "tr,tl")

   building = place_safe_building(player, "frisians_charcoal_burners_house", sf, 5, -14)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_clay_pit", sf, 7, -14)
   connected_road("normal", player, building.flag, "tr,r")

   building = place_safe_building(player, "frisians_clay_pit", sf, 9, -15)
   connected_road("normal", player, building.flag, "tr,tr,r")

   -- Building materials medium
   building = place_safe_building(player, "frisians_reed_farm", sf, 7, -10)
   connected_road("normal", player, building.flag, "l,l,l")
   connected_road("normal", player, building.flag, "tr,tl|tl,tr")

   building = place_safe_building(player, "frisians_reed_farm", sf, 3, -7)
   connected_road("normal", player, building.flag, "tr,tr,tr")
   connected_road("normal", player, building.flag, "l,bl")

   building = place_safe_building(player, "frisians_brick_kiln", sf, -1, -6)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_brick_kiln", sf, -4, -7)
   connected_road("normal", player, building.flag, "tr,r")

   building = place_safe_building(player, "frisians_weaving_mill", sf, -1, -3)
   connected_road("normal", player, building.flag, "tr,tr,tl")
   connected_road("normal", player, building.flag, "bl,bl,br")

   building = place_safe_building(player, "frisians_weaving_mill", sf, 4, -4)
   connected_road("normal", player, building.flag, "tr,tr|tl,tr|tl,tl")
   connected_road("normal", player, building.flag, "l,l,l")

   building = place_safe_building(player, "frisians_charcoal_kiln", sf, 1, -4)
   building = place_safe_building(player, "frisians_charcoal_kiln", sf, -4, 0)

   return count_buildings(player, total_previous_buildings, 16)
end

local function init_food_and_transport(player, sf, total_previous_buildings)
   print("============================================")
   print("Food and transport")
   print("--------------------------------------------")

   -- Food and transport small
   building = place_safe_building(player, "frisians_well", sf, -8, 0)
   building = place_safe_building(player, "frisians_well", sf, 4, 0)
   building = place_safe_building(player, "frisians_well", sf, -2, 11)

   -- (near water)
   building = place_safe_building(player, "frisians_fishers_house", sf, 3, 3)
   connected_road("normal", player, building.flag, "tr,tl,tl")

   building = place_safe_building(player, "frisians_fishers_house", sf, 2, 5)
   connected_road("normal", player, building.flag, "l,l,tl")

   building = place_safe_building(player, "frisians_fishers_house", sf, 3, 9)
   connected_road("normal", player, building.flag, "bl,bl|bl,bl")

   building = place_safe_building(player, "frisians_aqua_farm", sf, 5, 12)
   connected_road("normal", player, building.flag, "bl,bl,bl|bl,bl")

   -- (near animals)
   building = place_safe_building(player, "frisians_hunters_house", sf, -6, -10)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_berry_farm", sf, -8, -10)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_collectors_house", sf, -10, -10)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_beekeepers_house", sf, -12, -10)
   connected_road("normal", player, building.flag, "r,r")

   -- Food and transport medium
   building = place_safe_building(player, "frisians_smokery", sf, -2, 14)
   connected_road("normal", player, building.flag, "tr,tl,tl")

   building = place_safe_building(player, "frisians_bakery", sf, 0, 18)
   connected_road("normal", player, building.flag, "tr,tl,tl")

   building = place_safe_building(player, "frisians_honey_bread_bakery", sf, 3, 26)
   connected_road("normal", player, building.flag, "l,l")

   building = place_safe_building(player, "frisians_honey_bread_bakery", sf, 5, 26)
   connected_road("normal", player, building.flag, "l,l")

   building = place_safe_building(player, "frisians_brewery", sf, 7, 26)
   connected_road("normal", player, building.flag, "l,l")

   building = place_safe_building(player, "frisians_brewery", sf, 9, 26)
   connected_road("normal", player, building.flag, "l,l")

   building = place_safe_building(player, "frisians_mead_brewery", sf, -9, 22)
   connected_road("normal", player, building.flag, "r,tr,tr")

   building = place_safe_building(player, "frisians_mead_brewery", sf, -11, 22)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_tavern", sf, -13, 22)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_tavern", sf, -15, 22)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_drinking_hall", sf, -17, 22)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_drinking_hall", sf, -19, 22)
   connected_road("normal", player, building.flag, "r,r")


   -- Food and transport big
   building = place_safe_building(player, "frisians_farm", sf, -7, 20)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "frisians_farm", sf, -5, 16)
   connected_road("normal", player, building.flag, "tr,tr|tl,tr")

   building = place_safe_building(player, "frisians_farm", sf, -3, 21)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "frisians_farm", sf, -1, 17)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "frisians_farm", sf, -1, 25)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "frisians_farm", sf, 1, 21)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "frisians_farm", sf, 1, 26)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "frisians_farm", sf, 3, 22)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "frisians_reindeer_farm", sf, 7, 21)
   connected_road("normal", player, building.flag, "l,l,tl")

   building = place_safe_building(player, "frisians_reindeer_farm", sf, 9, 22)
   connected_road("normal", player, building.flag, "l,l,tl")
   connected_road("normal", player, building.flag, "tr,tr|r,tr")

   building = place_safe_building(player, "frisians_reindeer_farm", sf, 4, 15)
   building = place_safe_building(player, "frisians_reindeer_farm", sf, 2, 11)

   return count_buildings(player, total_previous_buildings, 35)
end

local function init_tools_and_training(player, sf, total_previous_buildings)
   print("============================================")
   print("Tools and training")
   print("--------------------------------------------")

   -- Tools, weapons and training sites medium
   building = place_safe_building(player, "frisians_furnace", sf, -6, 12)
   building = place_safe_building(player, "frisians_recycling_center", sf, -4, 12)

   building = place_safe_building(player, "frisians_blacksmithy", sf, -16, 18)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "frisians_armor_smithy_small", sf, -15, 16)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "frisians_armor_smithy_large", sf, -14, 18)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "frisians_sewing_room", sf, -13, 16)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "frisians_tailors_shop", sf, -12, 18)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "frisians_armor_smithy_large", sf, -11, 16)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "frisians_barracks", sf, -10, 18)
   connected_road("normal", player, building.flag, "tr,tr")

   building = place_safe_building(player, "frisians_training_camp", sf, -9, 16)
   connected_road("normal", player, building.flag, "tr,tr")


   -- Tools, weapons and training sites big
   building = place_safe_building(player, "frisians_training_arena", sf, -17, -5)
   connected_road("normal", player, building.flag, "bl,bl,l")
   connected_road("normal", player, building.flag, "tr,tr,tl,tr")

   building = place_safe_building(player, "frisians_training_arena", sf, -16, -9)
   connected_road("normal", player, building.flag, "tr,r,tr")

   building = place_safe_building(player, "frisians_training_arena", sf, -16, -1)
   connected_road("normal", player, building.flag, "tr,r|tr,r")

   building = place_safe_building(player, "frisians_training_arena", sf, -19, -3)
   connected_road("normal", player, building.flag, "br,br|r,r")

   building = place_safe_building(player, "frisians_training_arena", sf, -10, -3)
   building = place_safe_building(player, "frisians_training_arena", sf, -10, -6)
   connected_road("normal", player, building.flag, "br,bl,bl")

   return count_buildings(player, total_previous_buildings, 16)
end

local function init_mines(player, sf, total_previous_buildings)
   print("============================================")
   print("Mines")
   print("--------------------------------------------")

   -- Mines
   building = place_safe_building(player, "frisians_rockmine", sf, -8, 4)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")
   connected_road("normal", player, building.flag, "br,r")

   building = place_safe_building(player, "frisians_rockmine", sf, -6, 5)
   connected_road("normal", player, building.flag, "tr,tr,tr|tr,tr")

   building = place_safe_building(player, "frisians_rockmine_deep", sf, -7, 8)
   connected_road("normal", player, building.flag, "tr,tr,tl")

   building = place_safe_building(player, "frisians_rockmine_deep", sf, -9, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_coalmine", sf, -12, 1)
   connected_road("normal", player, building.flag, "tr,tr|tr,tr")

   building = place_safe_building(player, "frisians_coalmine", sf, -14, 2)
   connected_road("normal", player, building.flag, "tr,r")

   building = place_safe_building(player, "frisians_coalmine_deep", sf, -15, 3)
   connected_road("normal", player, building.flag, "r,tr")

   building = place_safe_building(player, "frisians_coalmine_deep", sf, -10, 3)
   connected_road("normal", player, building.flag, "l,tl,tl")

   building = place_safe_building(player, "frisians_ironmine", sf, -11, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_ironmine", sf, -13, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_ironmine_deep", sf, -15, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_ironmine_deep", sf, -12, 12)
   connected_road("normal", player, building.flag, "tr,tr,tr,tl")
   connected_road("normal", player, building.flag, "br,bl")

   building = place_safe_building(player, "frisians_goldmine", sf, -17, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_goldmine", sf, -19, 8)
   connected_road("normal", player, building.flag, "r,r")

   building = place_safe_building(player, "frisians_goldmine_deep", sf, -17, 4)
   connected_road("normal", player, building.flag, "r,tr")

   building = place_safe_building(player, "frisians_goldmine_deep", sf, -19, 4)
   connected_road("normal", player, building.flag, "r,r")
   connected_road("normal", player, building.flag, "l,l,tl")

   return count_buildings(player, total_previous_buildings, 16)
end


-- Place buildings for Frisian tribe
function init_frisians(player)
   assert_equal("frisians", player.tribe.name)

   local sf = wl.Game().map.player_slots[player.number].starting_field

   -- Headquarters
   place_warehouse(player, "frisians_headquarters", sf, 0, 0)

   local total_buildings = init_militarysites(player, sf, 1)
   total_buildings = init_warehouses(player, sf, total_buildings)
   total_buildings = init_shipconstruction(player, sf, total_buildings)
   total_buildings = init_buildingmaterials(player, sf, total_buildings)
   total_buildings = init_food_and_transport(player, sf, total_buildings)
   total_buildings = init_tools_and_training(player, sf, total_buildings)
   total_buildings = init_mines(player, sf, total_buildings)
   total_buildings = init_markets(player, sf, total_buildings)

   -- Construction
   player:place_building("frisians_training_arena", get_safe_field(sf, -2, 3), true, true)

   -- Test ships
   place_player_ship(player.number)
end

-- Run the tests
run(function()
   local playernumber = 4
   local expected_number_of_buildings = 124

   print("Placing buildings for Player 4")
   init_frisians(game.players[playernumber])

   -- Verify that nothing went wrong with placing the buildings,
   -- and that a building of each type has been placed.
   verify_buildings(playernumber, expected_number_of_buildings)

   -- Sleep a bit so the tribe units can run some programs
   sleep_with_fps(30 * 10)

   -- Test saveloading and verify the buildings again
   stable_save(game, "frisian_tribe", 1000)

   -- Did buildings saveload OK?
   verify_buildings(playernumber, expected_number_of_buildings)

   print("# All Tests passed.")
   mapview:close()
end)
