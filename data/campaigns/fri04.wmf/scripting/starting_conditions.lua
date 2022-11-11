local function place_milsite(p, name, x, y, soldier_levels, nr_soldiers)
   local b = p:place_building(name, map:get_field(x, y), false, true)
   b:set_soldiers(soldier_levels, nr_soldiers)
   b.destruction_blocked = true
end

-- =======================================================================
--                                 Player 1
-- =======================================================================
p1.team = 1
p1:allow_buildings("all")
p1:forbid_buildings {
   "frisians_port",
   "frisians_shipyard",
   "frisians_weaving_mill",
}
hq = p1:place_building("frisians_port", map.player_slots[1].starting_field, false, true)
hq:set_wares {
   brick = 20,
   log = 20,
   reed = 20,
   granite = 20,
   coal = 10,
   iron = 5,
   iron_ore = 10,
   gold_ore = 4,
   -- Leftover supplies from the journey
   ration = 10,
   smoked_meat = 5,
   smoked_fish = 5,
   bread_frisians = 5,
   honey_bread = 5,
   beer = 5,
   mead = 5,
   fruit = 10,
   honey = 5,
}
hq:set_workers {
   frisians_carrier = 40,
   frisians_builder = 5,
   frisians_blacksmith = 3,
   frisians_baker = 1,
   frisians_brewer = 1,
   frisians_claydigger = 2,
   frisians_brickmaker = 2,
   frisians_reed_farmer = 2,
   frisians_berry_farmer = 2,
   frisians_fruit_collector = 2,
   frisians_farmer = 2,
   frisians_landlady = 2,
   frisians_smoker = 2,
   frisians_geologist = 2,
   frisians_woodcutter = 3,
   frisians_forester = 2,
   frisians_miner = 4,
   frisians_smelter = 1,
   frisians_stonemason = 2,
}

local total_soldiers = 0
local soldiers = {50, 25, 5}
for descr,n in pairs(campaign_data.soldiers) do total_soldiers = total_soldiers + n end
if total_soldiers < soldiers[difficulty] then
   if campaign_data.soldiers["000"] then
      campaign_data.soldiers["000"] = campaign_data.soldiers["000"] + soldiers[difficulty] - total_soldiers
   else
      campaign_data.soldiers["000"] = soldiers[difficulty] - total_soldiers
   end
end
soldiers = {}
for descr,n in pairs(campaign_data.soldiers) do
   soldiers[{tonumber(descr:sub(1,1)), tonumber(descr:sub(2,2)), tonumber(descr:sub(3,3)), 0}] = n
end
hq:set_soldiers(soldiers)

-- =======================================================================
--                                 Player 2
-- =======================================================================
p2.hidden_from_general_statistics = true
p2:allow_buildings("all")
p2:forbid_buildings {
   "frisians_port",
   "frisians_shipyard",
   "frisians_weaving_mill",
}

hq = p2:place_building("frisians_headquarters", map.player_slots[2].starting_field, false, true)
hq:set_wares {
   brick = 50,
   log = 50,
   reed = 50,
   granite = 50,
   coal = 30,
   iron = 20,
   iron_ore = 20,
   gold = 10,
   gold_ore = 10,
   ration = 30,
   meal = 10,
   bread_frisians = 15,
   honey_bread = 5,
   beer = 15,
   mead = 5,
   fruit = 30,
   honey = 10,
   water = 20,
   barley = 30,
   clay = 20,
   fish = 50,
   meat = 5,
   smoked_meat = 60,
   smoked_fish = 20,
}
hq:set_workers {
   frisians_carrier = 40,
   frisians_builder = 5,
   frisians_blacksmith = 3,
   frisians_baker = 1,
   frisians_brewer = 1,
   frisians_claydigger = 2,
   frisians_brickmaker = 2,
   frisians_reed_farmer = 2,
   frisians_berry_farmer = 2,
   frisians_fruit_collector = 2,
   frisians_farmer = 2,
   frisians_landlady = 2,
   frisians_smoker = 2,
   frisians_geologist = 2,
   frisians_woodcutter = 3,
   frisians_forester = 2,
   frisians_miner = 4,
   frisians_stonemason = 2,
}
soldiers = {
   {{2,4,1,0}, 30},
   {{2,3,1,0}, 20},
   {{1,2,1,0}, 10},
}
hq:set_soldiers(soldiers[difficulty][1], soldiers[difficulty][2])

place_milsite(p2, "frisians_tower"   , 13,13, soldiers[difficulty][1],  7)
place_milsite(p2, "frisians_fortress",  0,16, soldiers[difficulty][1], 12)
place_milsite(p2, "frisians_fortress",  4,28, soldiers[difficulty][1], 12)

-- =======================================================================
--                                 Player 3
-- =======================================================================
if not campaign_data.payment then
   p3.hidden_from_general_statistics = true
   p3:forbid_buildings("all")
else
   p3:allow_buildings("all")
   p3:forbid_buildings {
      "empire_port",
      "empire_shipyard",
   }

   hq = p3:place_building("empire_port_large", map.player_slots[3].starting_field, false, true)
   local gold_iron_ratio = {80, 60, 40}
   hq:set_wares {
         gold = campaign_data.payment * gold_iron_ratio[difficulty] // 100,
         iron = campaign_data.payment * (100 - gold_iron_ratio[difficulty]) // 100,
         armor_helmet = 4,
         spear_wooden = 5,
         felling_ax = 6,
         bread_paddle = 2,
         basket = 2,
         empire_bread = 8,
         cloth = 5,
         coal = 12,
         fire_tongs = 2,
         fish = 6,
         fishing_rod = 2,
         flour = 4,
         grape = 4,
         hammer = 14,
         hunting_spear = 2,
         iron_ore = 5,
         kitchen_tools = 4,
         marble = 25,
         marble_column = 6,
         meal = 4,
         meat = 6,
         pick = 8,
         ration = 12,
         saw = 2,
         scythe = 5,
         shovel = 6,
         granite = 40,
         log = 30,
         water = 12,
         wheat = 4,
         wine = 8,
         planks = 45,
         wool = 2,
   }
   hq:set_workers {
         empire_armorsmith = 1,
         empire_brewer = 1,
         empire_builder = 10,
         empire_carrier = 40,
         empire_charcoal_burner = 1,
         empire_geologist = 4,
         empire_lumberjack = 3,
         empire_miner = 4,
         empire_stonemason = 2,
         empire_toolsmith = 2,
         empire_trainer = 3,
         empire_weaponsmith = 1,
         empire_donkey = 5,
   }
   soldiers = {
      {{1,2,0,1}, 70},
      {{2,3,0,2}, 80},
      {{4,4,0,2}, 90},
   }
   hq:set_soldiers(soldiers[difficulty][1], soldiers[difficulty][2])
end

-- =======================================================================
--                                 Player 4
-- =======================================================================
p4.hidden_from_general_statistics = true
p4:allow_buildings("all")
p4:forbid_buildings {
   "amazons_port",
   "amazons_shipyard",
}

hq = p4:place_building("amazons_headquarters", map.player_slots[4].starting_field, false, true)
hq:set_wares {
         log = 100,
         granite = 50,
         rope = 30,
         liana = 15,
         ironwood = 15,
         rubber = 15,
         balsa = 9,
         water = 20,
         fish = 10,
         meat = 10,
         cassavaroot = 15,
         ration = 5,
         coal = 5,
         gold = 4,
         amazons_bread = 5,
         chocolate = 6,
         hammer = 10,
         shovel = 5,
         cocoa_beans = 3,
         tunic = 5,
         chisel = 3,
         pick = 2,
         machete = 2,
         spear_wooden = 6,
         stone_bowl = 2,
         needles = 2,
}
hq:set_workers {
         amazons_cook = 3,
         amazons_builder = 10,
         amazons_gold_digger = 1,
         amazons_gold_smelter = 1,
         amazons_carrier = 40,
         amazons_cassava_farmer = 3,
         amazons_cocoa_farmer = 3,
         amazons_stone_carver = 3,
         amazons_geologist = 4,
         amazons_woodcutter = 10,
         amazons_liana_cutter = 4,
         amazons_jungle_preserver = 5,
         amazons_jungle_master = 3,
         amazons_shipwright = 1,
         amazons_hunter_gatherer = 3,
         amazons_stonecutter = 2,
         amazons_trainer = 3,
         amazons_dressmaker = 1,
         amazons_tapir = 5,
         amazons_woodcutter_master = 3,
}
if campaign_data.payment then
   soldiers = {
      {{2,2,1,2}, 45},
      {{2,2,1,2}, 55},
      {{2,2,1,2}, 65},
   }
else
   soldiers = {
      {{2,2,1,2}, 70},
      {{2,2,1,2}, 80},
      {{2,2,1,2}, 90},
   }
end
hq:set_soldiers(soldiers[difficulty][1], soldiers[difficulty][2])
place_milsite(p4, "amazons_fortification", 55,190, soldiers[difficulty][1], 12)
place_milsite(p4, "amazons_fortification", 47,186, soldiers[difficulty][1], 12)
place_milsite(p4, "amazons_fortification", 38,177, soldiers[difficulty][1], 12)
place_milsite(p4, "amazons_fortification", 22,173, soldiers[difficulty][1], 12)
