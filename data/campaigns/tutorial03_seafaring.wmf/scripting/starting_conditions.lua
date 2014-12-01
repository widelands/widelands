-- ===============
-- Initialization
-- ===============

plr:allow_buildings("all")

function init_player()
   filled_buildings(plr, {"port", sf.x, sf.y,
      wares = {
         diamond = 7,
         ironore = 20,
         quartz = 9,
         stone = 50,
         spideryarn = 9,
         log = 80,
         coal = 12,
         goldyarn = 6,
         iron = 12,
         planks = 65,
         spidercloth = 35,
         blackroot = 5,
         blackrootflour = 12,
         bread = 28,
         corn = 15,
         cornflour = 12,
         fish = 13,
         meat = 13,
         smoked_fish = 26,
         smoked_meat = 26,
         water = 12,
         bread_paddle = 2,
         bucket = 2,
         fire_tongs = 2,
         fishing_net = 4,
         hammer = 11,
         hunting_bow = 1,
         milking_tongs = 2,
         hook_pole = 2,
         pick = 8,
         saw = 9,
         scythe = 4,
         shovel = 9,
         tabard = 15,
         light_trident = 15,
      },
      workers = {
         armorsmith = 1,
         blackroot_farmer = 1,
         builder = 10,
         charcoal_burner = 1,
         carrier = 40,
         fish_breeder = 2,
         geologist = 4,
         miner = 3,
         sawyer = 1,
         stonecutter = 2,
         toolsmith = 2,
         weaponsmith = 1,
         woodcutter = 3,
         horse = 15,
      },
      soldiers = {
         [{0,0,0,0}] = 35,
      }
   },
   {"woodcutters_house", 32, 48},
   {"woodcutters_house", 33, 47},
   {"woodcutters_house", 29, 56},
   {"woodcutters_house", 30, 57},
   {"foresters_house", 31, 53},
   {"foresters_house", 32, 46},
   {"foresters_house", 32, 50},
   {"foresters_house", 34, 50},
   {"sawmill", 41, 46},
   {"quarry",  45, 54},
   {"tower", 43, 52},
   {"tower", 35, 46},
   {"guardhouse", 49, 60},
   {"crystalmine", 51, 61},
   {"guardhall", 43, 64},
   {"fish_breeders_house", 40, 66},
   {"fishers_house", 42, 66},
   {"fish_breeders_house", 44, 66},
   {"fishers_house", 46, 66},
   {"fish_breeders_house", 48, 66},
   {"castle", 22, 55},
   {"coalmine", 22, 48},
   {"coalmine", 16, 49},
   {"farm", 20, 59},
   {"farm", 23, 59},
   {"farm", 19, 64},
   {"farm", 21, 65},
   {"farm", 26, 64},
   {"blackroot_farm", 53, 53},
   {"blackroot_farm", 56, 50},
   {"mill", 34, 58},
   {"mill", 32, 64},
   {"bakery", 38, 49},
   {"bakery", 40, 48},
   {"smokery", 46, 57},
   {"smokery", 44, 57},
   {"horsefarm", 40, 55},
   {"spiderfarm", 37, 45},
   {"weaving-mill", 45, 45},
   {"smelting_works", 35, 56, wares = {coal = 8, ironore = 8}}, -- no gold
   {"smelting_works", 35, 59, wares = {coal = 8, ironore = 8}},
   {"toolsmithy", 41, 52},
   {"weaponsmithy", 37, 54},
   {"small_tower", 34, 63},
   {"tower", 52, 46},
   {"well", 39, 43},
   {"well", 39, 45},
   {"well", 38, 55},
   {"well", 35, 61},
   {"well", 32, 59},
   {"warehouse", 36, 57},

   -- everthing below is in the north
   {"castle", castle_field.x, castle_field.y},
   {"warehouse", 37, 16,
      wares = {
         smoked_meat = 50,
         smoked_fish = 150,
         bread = 200,
         log = 30,
         planks = 30,
         stone = 10,
         diamond = 3,
         quartz = 4,
         spidercloth = 14,
         gold = 2,
         hammer = 1
      },
      workers = {
         builder = 3,
         miner = 3
      },
      soldiers = {
         [{0,0,0,0}] = 10,
      }
   }, -- end of warehouse on northern peninsula
   {"goldmine", 32, 25}
   )

   plr:forbid_buildings{"shipyard"}

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

