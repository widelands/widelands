-- ===============
-- Initialization
-- ===============

plr:allow_buildings("all")

function init_player()
   filled_buildings(plr, {"atlanteans_port", sf.x, sf.y,
      wares = {
         blackroot = 5,
         blackroot_flour = 12,
         atlanteans_bread = 28,
         bread_paddle = 2,
         buckets = 2,
         coal = 12,
         corn = 15,
         cornmeal = 12,
         diamond = 7,
         fire_tongs = 2,
         fish = 13,
         fishing_net = 4,
         gold_thread = 6,
         granite = 50,
         hammer = 11,
         hook_pole = 2,
         hunting_bow = 1,
         iron = 12,
         iron_ore = 20,
         log = 80,
         meat = 13,
         milking_tongs = 2,
         pick = 8,
         planks = 65,
         quartz = 9,
         saw = 9,
         scythe = 4,
         shovel = 9,
         smoked_fish = 26,
         smoked_meat = 26,
         spider_silk = 9,
         spidercloth = 35,
         tabard = 15,
         trident_light = 15,
         water = 12,
      },
      workers = {
         atlanteans_armorsmith = 1,
         atlanteans_blackroot_farmer = 1,
         atlanteans_builder = 10,
         atlanteans_charcoal_burner = 1,
         atlanteans_carrier = 40,
         atlanteans_fishbreeder = 2,
         atlanteans_geologist = 4,
         atlanteans_miner = 3,
         atlanteans_sawyer = 1,
         atlanteans_stonecutter = 2,
         atlanteans_toolsmith = 2,
         atlanteans_weaponsmith = 1,
         atlanteans_woodcutter = 3,
         atlanteans_horse = 15,
      },
      soldiers = {
         [{0,0,0,0}] = 35,
      }
   },
   {"atlanteans_woodcutters_house", 32, 48},
   {"atlanteans_woodcutters_house", 33, 47},
   {"atlanteans_woodcutters_house", 29, 56},
   {"atlanteans_woodcutters_house", 30, 57},
   {"atlanteans_foresters_house", 31, 53},
   {"atlanteans_foresters_house", 32, 46},
   {"atlanteans_foresters_house", 32, 50},
   {"atlanteans_foresters_house", 34, 50},
   {"atlanteans_sawmill", 41, 46},
   {"atlanteans_quarry",  45, 54},
   {"atlanteans_tower", 43, 52},
   {"atlanteans_tower", 35, 46},
   {"atlanteans_guardhouse", 49, 60},
   {"atlanteans_crystalmine", 51, 61},
   {"atlanteans_guardhall", 43, 64},
   {"atlanteans_fishbreeders_house", 40, 66},
   {"atlanteans_fishers_house", 42, 66},
   {"atlanteans_fishbreeders_house", 44, 66},
   {"atlanteans_fishers_house", 46, 66},
   {"atlanteans_fishbreeders_house", 48, 66},
   {"atlanteans_castle", 22, 55},
   {"atlanteans_coalmine", 22, 48},
   {"atlanteans_coalmine", 16, 49},
   {"atlanteans_farm", 20, 59},
   {"atlanteans_farm", 23, 59},
   {"atlanteans_farm", 19, 64},
   {"atlanteans_farm", 21, 65},
   {"atlanteans_farm", 26, 64},
   {"atlanteans_blackroot_farm", 53, 53},
   {"atlanteans_blackroot_farm", 56, 50},
   {"atlanteans_mill", 34, 58},
   {"atlanteans_mill", 32, 64},
   {"atlanteans_bakery", 38, 49},
   {"atlanteans_bakery", 40, 48},
   {"atlanteans_smokery", 46, 57},
   {"atlanteans_smokery", 44, 57},
   {"atlanteans_horsefarm", 40, 55},
   {"atlanteans_spiderfarm", 37, 45},
   {"atlanteans_weaving_mill", 45, 45},
   {"atlanteans_smelting_works", 35, 56, wares = {coal = 8, iron_ore = 8}}, -- no gold
   {"atlanteans_smelting_works", 35, 59, wares = {coal = 8, iron_ore = 8}},
   {"atlanteans_toolsmithy", 41, 52},
   {"atlanteans_weaponsmithy", 37, 54},
   {"atlanteans_tower_small", 34, 63},
   {"atlanteans_tower", 52, 46},
   {"atlanteans_well", 39, 43},
   {"atlanteans_well", 39, 45},
   {"atlanteans_well", 38, 55},
   {"atlanteans_well", 35, 61},
   {"atlanteans_well", 32, 59},
   {"atlanteans_warehouse", 36, 57},

   -- everthing below is in the north
   {"atlanteans_castle", castle_field.x, castle_field.y},
   {"atlanteans_warehouse", 37, 16,
      wares = {
         atlanteans_bread = 200,
         diamond = 3,
         gold = 2,
         granite = 10,
         hammer = 1,
         log = 30,
         planks = 30,
         quartz = 4,
         smoked_fish = 150,
         smoked_meat = 50,
         spidercloth = 14,
      },
      workers = {
         atlanteans_builder = 3,
         atlanteans_miner = 3
      },
      soldiers = {
         [{0,0,0,0}] = 10,
      }
   }, -- end of warehouse on northern peninsula
   {"atlanteans_goldmine", 32, 25}
   )

   plr:forbid_buildings{"atlanteans_shipyard"}

   connected_road(plr,map:get_field(42,44).immovable,"bl,br,bl|bl,bl|bl,l|l,bl|l,l,l|l,l|tr,tl|r,tr")
   connected_road(plr,map:get_field(42,44).immovable,"br,r|br,r,r|r,r,tr|r,r|r,r|br,bl|br,br|br,br|r,r")
   connected_road(plr,map:get_field(35,47).immovable,"r,r,tr|r,r|br,r|br,br|br,br|br,bl|br,br,r|br,r|br,bl|br,br|br,r|r,br,r")
   connected_road(plr,map:get_field(45,58).immovable,"r,r|bl,br,bl|bl,bl,l|bl,bl|bl,bl")
   connected_road(plr,map:get_field(48,67).immovable,"l,l|l,l|l,l|l,l|l,tl|tl,tl|l,l,l|l,l,bl|l,tl,l|l,bl|l,l|bl,l,l|l,l|l,l,tl|tr,tr|tl,tr,tr|tr,tr|tr,tr|tr,tr|tr,tr|tr,tr|tr,tr|tr,tr|tr,tr|tr,r|r,r|r,r|br,br|br,r,r")
   connected_road(plr,map:get_field(43,53).immovable,"l,l|br,bl,bl|l,l|l,tl|bl,l,bl|bl,bl|br,r|br,bl|bl,bl")
   connected_road(plr,map:get_field(17,50).immovable,"r,r|r,r|tr,r|br,r|br,br|r,br|br,br|br,br,r|br,r|bl,br|r,r|tr,r")
   connected_road(plr,map:get_field(24,60).immovable,"l,l,l")
   connected_road(plr,map:get_field(54,54).immovable,"tr,tr,tl")
   connected_road(plr,map:get_field(42,44).immovable,"l,l|l,tl|l,l|l,l")
   connected_road(plr,map:get_field(37,58).immovable,"bl,bl")
   connected_road(plr,map:get_field(41,47).immovable,"r,tr,tr")

   connected_road(plr,map:get_field(33,26).immovable,"r,tr|tr,tr|tr,tr|tr,tr|tl,tr")
end

run(init_player)
