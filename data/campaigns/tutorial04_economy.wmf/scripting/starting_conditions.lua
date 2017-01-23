-- ===============
-- Initialization
-- ===============

plr:forbid_buildings{"empire_scouts_house"} -- otherwise, player could scout the wasteland

function init_player()
   -- a headquarters without helms, spears, ores and coal
   prefilled_buildings(plr, { "empire_headquarters", sf.x, sf.y,
      wares = {
         felling_ax = 6,
         bread_paddle = 2,
         basket = 2,
         empire_bread = 28,
         cloth = 5,
         fire_tongs = 2,
         fish = 6,
         fishing_rod = 2,
         flour = 34,
         grape = 4,
         hammer = 14,
         hunting_spear = 2,
         kitchen_tools = 4,
         marble = 25,
         marble_column = 6,
         meal = 4,
         meat = 6,
         pick = 8,
         ration = 5,
         saw = 2,
         scythe = 5,
         shovel = 6,
         granite = 40,
         log = 60,
         water = 12,
         wheat = 24,
         wine = 8,
         planks = 45,
         wool = 2,
      },
      workers = {
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
         empire_weaponsmith = 1,
         empire_donkey = 5,
      }
   })

   filled_buildings_one_soldier(plr,
      {"empire_lumberjacks_house",92,65},
      {"empire_lumberjacks_house",89,52},
      {"empire_foresters_house",90,54},
      {"empire_foresters_house",91,56},
      {"empire_lumberjacks_house",92,57},
      {"empire_foresters_house",93,59},
      {"empire_lumberjacks_house",93,61},
      {"empire_quarry",99,57},
      {"empire_quarry",89,38},
      {"empire_sawmill",91,53},
      {"empire_stonemasons_house",92,50},
      {"empire_donkeyfarm",96,58},
      {"empire_farm",88,48},
      {"empire_farm",91,38},
      {"empire_farm",94,39},
      {"empire_farm",88,34},
      {"empire_vineyard",97,49},
      {"empire_vineyard",100,50},
      {"empire_vineyard",103,50},
      {"empire_vineyard",103,56},
      {"empire_winery",101,52},
      {"empire_winery",101,57},
      {"empire_brewery",106,49},
      {"empire_brewery",104,49},
      {"empire_piggery",110,23},
      {"empire_piggery",113,21},
      {"empire_warehouse",110,29,
         wares = {
            wheat = 20,
            flour = 20
         }
      },
      {"empire_mill",105,46},
      {"empire_mill",107,44},
      {"empire_mill",108,39},
      {"empire_bakery",114,26},
      {"empire_bakery",116,28},
      {"empire_bakery",115,32},
      {"empire_tavern",tavern_field.x,tavern_field.y}, -- (105,44), will be destroyed
      {"empire_coalmine",118,45, inputs = {beer = 6}},
      {"empire_coalmine",119,39, inputs = {beer = 6}},
      {"empire_ironmine",107,59, inputs = {beer = 6}},
      {"empire_marblemine",98,38, inputs = {wine = 6}},
      {"empire_marblemine",102,38, inputs = {wine = 6}},
      {"empire_smelting_works",110,38, inputs = {}},
      {"empire_smelting_works",111,43, inputs = {}},
      {"empire_toolsmithy",104,64, inputs = {log = 8}},
      {"empire_weaponsmithy",113,40, inputs = {planks = 8}},
      {"empire_armorsmithy",112,37, inputs = {cloth = 8}},
      {"empire_farm",105,70},
      {"empire_farm",101,71},
      {"empire_farm",99,77},
      {"empire_fishers_house",106,77},
      {"empire_fishers_house",104,77},
      {"empire_fishers_house",103,79},
      {"empire_warehouse",100,74},
      {"empire_hunters_house",95,60},
      {"empire_well",92,48},
      {"empire_well",103,45},
      {"empire_well",107,40},
      {"empire_well",103,47},
      {"empire_well",104,51},
      {"empire_well",106,51},
      {"empire_well",113,23},
      {"empire_well",113,32},
      {"empire_well",112,34},
      {"empire_well",111,31},
      {"empire_well",115,24},
      {"empire_warehouse",warehouse_field.x,warehouse_field.y}, -- (78,67)

      {"empire_blockhouse",89,31},
      {"empire_barrier",91,45},
      {"empire_tower",103,42},
      {"empire_barrier",112,25},
      {"empire_sentry",116,41},
      {"empire_blockhouse",102,66},
      {"empire_outpost",103,76},
      {"empire_tower",90,67},
      {"empire_sentry",105,56},
      {"empire_fortress",66,63},
      {"empire_fortress",71,66},
      {"empire_fortress",75,72}
   )
   plr:place_building("empire_quarry", map:get_field(87,36), true, true) -- a construction site

   plr:conquer(map:get_field(111,34),3) -- some remaining fields inside

   connected_road(plr,map:get_field(97,54).immovable,"tr,tr|tr,tl")
   connected_road(plr,map:get_field(98,52).immovable,"br,r|r,r|tr,r,tr|tr,r|r,r|tr,tr|tr,tr|tr,tr|r,r|r,r|r,r|br,br|r,r")
   connected_road(plr,map:get_field(99,53).immovable,"tr,tr|tr,tr|tl,tr|tr,tr|tr,tr|tr,tl|tl,tl|l,l|l,l|l,l|l,bl")
   connected_road(plr,map:get_field(97,54).immovable,"bl,br|br,bl,bl|bl,bl|bl,l|bl,bl|br,bl|bl,bl,l|l,l|l,l|l,l|l,l|l,l|l,l|l,l|l,l|l,l|l,tl|l,l|l,tl|tl,tl")
   connected_road(plr,map:get_field(97,56).immovable,"r,br|br,r|r,r|r,tr|r,r|br,br|br,r")
   connected_road(plr,map:get_field(97,54).immovable,"l,l|l,tl|tl,tl|bl,l|l,bl")
   connected_road(plr,map:get_field(88,49).immovable,"br,br|r,br")
   connected_road(plr,map:get_field(92,51).immovable,"tr,tl|tr,tl,tl|tr,tr|tr,tr|tr,tr")
   connected_road(plr,map:get_field(94,42).immovable,"tl,l|tl,tl|l,l|l,tl,tl|tr,tr|tr,tr,tr")
   connected_road(plr,map:get_field(75,73).immovable,"tr,tr|tr,r|tr,tr")
   connected_road(plr,map:get_field(92,54).immovable,"tr,r")
   connected_road(plr,map:get_field(93,58).immovable,"tr,tr|tr,tr")
   connected_road(plr,map:get_field(102,45).immovable,"br,r|br,r|br,bl,bl")
   connected_road(plr,map:get_field(105,47).immovable,"tr,tl|r,r|br,r")
   connected_road(plr,map:get_field(116,44).immovable,"tr,tr|tr,tr|r,r")
   connected_road(plr,map:get_field(105,45).immovable,"tr,tr|tr,tr|r,tr|tr,r|tr,tr|tr,tr|tr,tr|r,r|tr,tr|tr,tl|l,tl,tl|l,tl|l,tl,tl")
   connected_road(plr,map:get_field(114,22).immovable,"br,bl|bl,bl|bl,bl|bl,bl|bl,bl|bl,bl|bl,bl|bl,bl|bl,bl|bl,bl|bl,l")
   connected_road(plr,map:get_field(105,42).immovable,"br,r")
   connected_road(plr,map:get_field(111,37).immovable,"br,r|r,br|bl,bl|bl,bl,bl")
   connected_road(plr,map:get_field(106,59).immovable,"bl,bl|bl,br|bl,bl|l,bl,bl|bl,br|br,br|r,r|br,bl|br,br|br,br,bl|l,l|bl,bl|l,tl|tl,tl|tl,tl|tr,tr,tr|tr,r")
   connected_road(plr,map:get_field(100,78).immovable,"tr,r|r,r")
   connected_road(plr,map:get_field(104,48).immovable,"r,tr")
   connected_road(plr,map:get_field(105,52).immovable,"tr,tl")
   connected_road(plr,map:get_field(107,52).immovable,"tr,tl")
   connected_road(plr,map:get_field(112,32).immovable,"l,l")
   connected_road(plr,map:get_field(115,25).immovable,"bl,bl")
   connected_road(plr,map:get_field(97,54).immovable,"r,r,tr")
end
