-- =======================================================================
--                                 Player 1
-- =======================================================================
p1:allow_buildings("all")
p1:forbid_buildings({"frisians_quarry", "frisians_hunters_house", "frisians_tavern", "frisians_coalmine", "frisians_ironmine", "frisians_goldmine", "frisians_rockmine"})
-- NOCOM also forbid frisians_dikers_house
include "map:scripting/init_p1.lua"

local soldiers = {}
for descr,n in pairs(campaign_data) do
   soldiers[{tonumber(descr:sub(1,1)), tonumber(descr:sub(2,2)), tonumber(descr:sub(3,3)), 0}] = n
end
p1:get_buildings("frisians_headquarters")[1]:set_soldiers(soldiers)

-- =======================================================================
--                                 Player 2
-- =======================================================================
p2:allow_buildings("all")
p2:forbid_buildings({"empire_quarry", "empire_hunters_house", "empire_tavern", "empire_coalmine", "empire_ironmine", "empire_goldmine", "empire_marblemine", "empire_vineyard", "empire_winery"})
hq = p2:place_building("empire_port", map.player_slots[2].starting_field, false, true)
hq:set_wares {
   log = 80,
   granite = 40,
   marble = 80,
   marble_column = 20,
   planks = 40,
   cloth = 10,
   beer = 15,
   meal = 5,
   wheat = 30,
   water = 50,
   coal = 250,
   iron = 50,
   gold = 50,
   gold_ore = 250,
   iron_ore = 500,
   meat = 20,
   fish = 20,
   empire_bread = 20
}
hq:set_workers {
   empire_builder = 3,
   empire_lumberjack = 4,
   empire_forester = 8,
   empire_stonemason = 4,
   empire_toolsmith = 1,
   empire_carpenter = 1,
   empire_donkey = 30,
   empire_donkeybreeder = 1,
   empire_weaver = 2,
   empire_fisher = 2,
   empire_shepherd = 2,
   empire_farmer = 9,
   empire_miller = 4,
   empire_baker = 4,
   empire_brewer = 4,
   empire_pigbreeder = 3,
   empire_charcoal_burner = 2,
   empire_smelter = 4,
   empire_trainer = 3,
   empire_armorsmith = 1,
   empire_weaponsmith = 1,
}
local soldiers = {70, 110, 150}
hq:set_soldiers({4,4,0,2}, soldiers[difficulty])
