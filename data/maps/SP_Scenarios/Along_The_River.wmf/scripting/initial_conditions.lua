-- =======================================================================
--                             Along The River
-- =======================================================================

-- =======================================================================
--                                 Player 1
-- =======================================================================
p1:allow_buildings("all")
p1:forbid_buildings{"barbarians_shipyard"}

prefilled_buildings(p1,
   { "barbarians_headquarters_interim", 15, 119,
      wares = {
         log = 80,
         blackwood = 32,
         granite = 40,
         grout = 12,
         reed = 24,
         cloth = 5,
         coal = 12,
         iron_ore = 5,
         iron = 12,
         gold = 8,
         fish = 6,
         meat = 6,
         barbarians_bread = 8,
         ration = 12,
         snack = 3,
         meal = 4,
         pick = 14,
         shovel = 4,
         felling_ax = 4,
         fishing_rod = 3,
         hunting_spear = 3,
         scythe = 6,
         hammer = 12,
         fire_tongs = 2,
         bread_paddle = 2,
         kitchen_tools = 4,
         ax = 6,
      },
      workers = {
         barbarians_carrier = 40,
         barbarians_ox = 20,
         barbarians_stonemason = 2,
         barbarians_lumberjack = 4,
         barbarians_ranger = 2,
         barbarians_fisher = 2,
         barbarians_builder = 10,
         barbarians_brewer = 1,
         barbarians_blacksmith = 2,
         barbarians_geologist = 6,
         barbarians_miner = 4,
      },
      soldiers = {
         [{0,0,0,0}] = 30,
      },
   },
   { "barbarians_sentry", 22, 101, soldiers = { [{0,0,0,0}] = 2 }},
   { "barbarians_sentry", 27, 128, soldiers = { [{0,0,0,0}] = 2 }},
   { "barbarians_scouts_hut", 23, 103, inputs = {ration = 2} },
   { "barbarians_scouts_hut", 28, 129, inputs = {ration = 2} },

   { "barbarians_battlearena", 12, 119, inputs = {barbarians_bread = 8, fish = 6, meat = 6, beer_strong = 8} },
   { "barbarians_trainingcamp", 14, 115, inputs = {barbarians_bread = 10, fish = 6, meat = 6} },
   { "barbarians_helmsmithy", 16, 111, inputs = {iron = 8, gold = 8, coal = 8} },

   { "barbarians_well", 16, 117},
   { "barbarians_metal_workshop", 17, 115, inputs = {iron = 8, log = 8} },
   { "barbarians_smelting_works", 18, 113, inputs = {iron_ore = 8, gold_ore = 8, coal = 8} },
   { "barbarians_barrier", 19, 111, soldiers = { [{0,0,0,0}] = 5 }},
   { "barbarians_reed_yard", 20, 109},

   { "barbarians_wood_hardener", 17, 119, inputs = {log = 8} },
   { "barbarians_lime_kiln", 18, 117, inputs = {granite = 6, water= 6, coal = 3} },
   { "barbarians_ax_workshop", 19, 115, inputs = {iron = 8, coal = 8} }
)

-- =======================================================================
                                -- Player 2
-- =======================================================================
p2:forbid_buildings("all")
p2:allow_buildings{
   "barbarians_coalmine_deep",
   "barbarians_coalmine_deeper",
   "barbarians_ironmine_deep",
   "barbarians_ironmine_deeper",
   "barbarians_goldmine_deep",
   "barbarians_goldmine_deeper",
   "barbarians_quarry",
   "barbarians_lumberjacks_hut",
   "barbarians_rangers_hut",
   "barbarians_fishers_hut",
   "barbarians_hunters_hut",
   "barbarians_gamekeepers_hut",
   "barbarians_well",
   "barbarians_warehouse",
   "barbarians_wood_hardener",
   "barbarians_lime_kiln",
   "barbarians_reed_yard",
   "barbarians_bakery",
   "barbarians_micro_brewery",
   "barbarians_brewery",
   "barbarians_tavern",
   "barbarians_inn",
   "barbarians_big_inn",
   "barbarians_charcoal_kiln",
   "barbarians_smelting_works",
   "barbarians_metal_workshop",
   "barbarians_ax_workshop",
   "barbarians_warmill",
   "barbarians_cattlefarm",
   "barbarians_farm",
   "barbarians_helmsmithy",
   "barbarians_trainingcamp"
}

prefilled_buildings(p2,
   { "barbarians_headquarters_interim", 33, 17,
      wares = {
         log = 80,
         blackwood = 32,
         granite = 40,
         grout = 12,
         reed = 24,
         coal = 12,
         iron_ore = 5,
         iron = 12,
         gold = 8,
         fish = 6,
         meat = 6,
         barbarians_bread = 8,
         ration = 12,
         snack = 3,
         meal = 4,
         pick = 14,
         shovel = 4,
         fishing_rod = 3,
         hunting_spear = 2,
         scythe = 6,
         hammer = 12,
         fire_tongs = 2,
         bread_paddle = 2,
         kitchen_tools = 4,
         ax = 6,
      },
      workers = {
         barbarians_carrier = 40,
         barbarians_ox = 20,
         barbarians_stonemason = 2,
         barbarians_lumberjack = 3,
         barbarians_ranger = 1,
         barbarians_fisher = 2,
         barbarians_builder = 10,
         barbarians_lime_burner = 1,
         barbarians_gardener = 1,
         barbarians_brewer = 1,
         barbarians_blacksmith = 2,
         barbarians_geologist = 8,
         barbarians_miner = 4
      }
   },
   { "barbarians_warehouse", 26, 59,
      wares = {log = 20,blackwood=20,granite=10,grout=5,reed=5},
      workers = {
         barbarians_carrier = 40,
         barbarians_ox = 20,
         barbarians_builder = 4
   }},
   { "barbarians_warehouse", 30, 14   },
   { "barbarians_warehouse", 39, 16   },
   { "barbarians_fortress", 27, 76, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_barrier",  21, 81, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_barrier",  22, 74, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_barrier",  24, 60, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_barrier",  32, 56, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_barrier",  35, 54, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_barrier",  32, 46, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_sentry",   24, 81, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_sentry",   22, 63, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_tower",   25, 69, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_fortress", 29, 59, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_citadel",  33, 50, soldiers = { [{1,1,0,0}] = 1 }},

   { "barbarians_barrier",  40, 50, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_fortress", 50, 41, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_fortress", 53, 30, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_barrier",  43, 33, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_fortress", 35, 31, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_barrier",  28, 28, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_fortress", 25, 18, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_barrier",  30, 10, soldiers = { [{1,1,0,0}] = 1 }},
   { "barbarians_fortress", 42, 17, soldiers = { [{1,1,0,0}] = 1 }},

   { "barbarians_rangers_hut", 31, 17 },
   { "barbarians_rangers_hut", 28, 64 },
   { "barbarians_rangers_hut", 21, 66 },

   { "barbarians_ironmine", 19, 21, inputs = { ration = 6 }},
   { "barbarians_coalmine", 21, 15, inputs = { ration = 6 }},
   { "barbarians_coalmine", 20, 13, inputs = { ration = 6 }},
   { "barbarians_goldmine", 23, 11, inputs = { ration = 6 }},
   { "barbarians_granitemine", 26, 7, inputs = { ration = 8 }},
   { "barbarians_granitemine", 30, 4, inputs = { ration = 8 }}
)

-- =======================================================================
                                -- Player 4
-- =======================================================================
p4:forbid_buildings("all")
p4:allow_buildings{
   "barbarians_coalmine_deep",
   "barbarians_coalmine_deeper",
   "barbarians_ironmine_deep",
   "barbarians_ironmine_deeper",
   "barbarians_goldmine_deep",
   "barbarians_goldmine_deeper",
   "barbarians_quarry",
   "barbarians_lumberjacks_hut",
   "barbarians_rangers_hut",
   "barbarians_fishers_hut",
   "barbarians_hunters_hut",
   "barbarians_gamekeepers_hut",
   "barbarians_well",
   "barbarians_warehouse",
   "barbarians_wood_hardener",
   "barbarians_lime_kiln",
   "barbarians_reed_yard",
   "barbarians_bakery",
   "barbarians_micro_brewery",
   "barbarians_brewery",
   "barbarians_tavern",
   "barbarians_inn",
   "barbarians_big_inn",
   "barbarians_charcoal_kiln",
   "barbarians_smelting_works",
   "barbarians_metal_workshop",
   "barbarians_ax_workshop",
   "barbarians_warmill",
   "barbarians_cattlefarm",
   "barbarians_farm",
   "barbarians_helmsmithy",
   "barbarians_battlearena",
   "barbarians_trainingcamp"
}

-- Room for mines.
p4:conquer(wl.Game().map:get_field(107, 60), 3)
p4:conquer(wl.Game().map:get_field(105, 63), 3)

prefilled_buildings(p4,
   { "barbarians_headquarters_interim", 102, 36,
      wares = {
         log = 80,
         blackwood = 32,
         granite = 40,
         grout = 12,
         reed = 24,
         coal = 12,
         iron_ore = 6,
         iron = 12,
         gold_ore = 5,
         gold = 8,
         fish = 6,
         meat = 6,
         barbarians_bread = 8,
         pick = 14,
         shovel = 4,
         felling_ax = 4,
         fishing_rod = 2,
         hunting_spear = 2,
         scythe = 6,
         hammer = 12,
         fire_tongs = 2,
         bread_paddle = 2,
         kitchen_tools = 4,
         ax = 6,
      },
      workers = {
         barbarians_carrier = 40,
         barbarians_ox = 20,
         barbarians_stonemason = 2,
         barbarians_lumberjack = 3,
         barbarians_ranger = 1,
         barbarians_fisher = 2,
         barbarians_builder = 10,
         barbarians_lime_burner = 1,
         barbarians_gardener = 1,
         barbarians_brewer = 1,
         barbarians_brewer_master = 1,
         barbarians_blacksmith = 2,
         barbarians_blacksmith_master = 1,
         barbarians_geologist = 6,
         barbarians_miner = 4,
      }
   },

   { "barbarians_warehouse", 112, 30,
      wares = {log = 80,blackwood=40,granite=40,grout=20,reed=25},
      workers = {
         barbarians_carrier = 40,
         barbarians_ox = 20,
         barbarians_builder = 4
   }},

   { "barbarians_warehouse", 97, 23,
      wares = {log = 80,blackwood=40,granite=40,grout=20,reed=25},
      workers = {
         barbarians_carrier = 40,
         barbarians_ox = 20,
         barbarians_builder = 4
   }},

   { "barbarians_coalmine",          105,  63, inputs = {ration = 6 }},
   { "barbarians_coalmine",          107,  60, inputs = {ration = 6 }},
   { "barbarians_ironmine",            109,  57, inputs = {ration = 6 }},
   { "barbarians_ironmine",            112,  56, inputs = {ration = 6 }},
   { "barbarians_granitemine",      115,  50, inputs = {ration = 8 }},
   { "barbarians_granitemine",      117,  46, inputs = {ration = 8 }},
   { "barbarians_goldmine",           81,   0, inputs = {ration = 6 }},
   { "barbarians_barrier",    106,  50, soldiers = { [{2,3,0,0}] = 1 }},   -- close to black
   { "barbarians_sentry",    102,  48, soldiers = { [{2,3,0,0}] = 1 }},   -- close to black
   { "barbarians_fortress", 110,  48, soldiers = { [{2,3,0,0}] = 1 }},
   { "barbarians_barrier",   96,  36, soldiers = { [{2,3,0,0}] = 1 }},   -- close to black
   { "barbarians_fortress", 119,  30, soldiers = { [{2,3,0,0}] = 1 }},
   { "barbarians_fortress", 129,  20, soldiers = { [{2,3,0,0}] = 1 }},
   { "barbarians_fortress", 138,  20, soldiers = { [{2,3,0,0}] = 1 }},
   { "barbarians_fortress",  99,  12, soldiers = { [{2,3,0,0}] = 1 }},
   { "barbarians_fortress", 114,  40, soldiers = { [{2,3,0,0}] = 1 }},
   { "barbarians_barrier",     91,  14, soldiers = { [{2,3,0,0}] = 1 }},
   { "barbarians_fortress",   103,  22, soldiers = { [{2,3,0,0}] = 1 }},
   { "barbarians_sentry",     93,  22, soldiers = { [{2,3,0,0}] = 1 }},
   { "barbarians_barrier",     83,   8, soldiers = { [{2,3,0,0}] = 1 }},
   { "barbarians_barrier",     76,   2, soldiers = { [{2,3,0,0}] = 1 }}
)

-- =======================================================================
                                -- Player 5
-- =======================================================================

p5:forbid_buildings("all")
p5:allow_buildings{
   "barbarians_fishers_hut",
   "barbarians_hunters_hut",
   "barbarians_gamekeepers_hut",
   "barbarians_well"
}
p5:conquer(wl.Game().map:get_field(60, 69), 5)
prefilled_buildings(p5,
   { "barbarians_headquarters_interim", 87, 65,
      wares = {
         log = 80,
         granite = 40,
         hammer = 3,
         shovel = 5,
         felling_ax = 5,
         fishing_rod = 7,
         hunting_spear = 5
      },
      workers = {
         barbarians_carrier = 70,
         barbarians_ox = 5
      }
   },

   { "barbarians_headquarters_interim", 134, 65,
      wares = {
         log = 80,
         granite = 40,
         hammer = 3,
         shovel = 5,
         felling_ax = 5,
         fishing_rod = 7,
         hunting_spear = 5
      },
      workers = {
         barbarians_carrier = 70,
         barbarians_ox = 5
      }
   },

   { "barbarians_tavern", 92, 72 },
   { "barbarians_tavern", 128, 66 },

   { "barbarians_lumberjacks_hut", 134, 56 },
   { "barbarians_rangers_hut", 135, 54 },

   { "barbarians_fishers_hut", 82, 60 },
   { "barbarians_fishers_hut", 78, 65 },
   { "barbarians_fishers_hut", 72, 70 },
   { "barbarians_fishers_hut", 128, 73 },
   { "barbarians_fishers_hut", 138, 68 },
   { "barbarians_fishers_hut", 141, 52 },
   { "barbarians_fishers_hut", 100, 96 },

   { "barbarians_hunters_hut", 93, 66 },
   { "barbarians_hunters_hut", 84, 69 },
   { "barbarians_hunters_hut", 83, 93 },
   { "barbarians_hunters_hut", 78, 73 },
   { "barbarians_hunters_hut", 77, 89 },
   { "barbarians_hunters_hut", 68, 88 },
   { "barbarians_hunters_hut", 102, 93 },
   { "barbarians_hunters_hut", 115, 85 },
   { "barbarians_hunters_hut", 132, 62 },

   { "barbarians_gamekeepers_hut", 93, 70 },
   { "barbarians_gamekeepers_hut", 86, 70 },
   { "barbarians_gamekeepers_hut", 85, 92 },
   { "barbarians_gamekeepers_hut", 79, 70 },
   { "barbarians_gamekeepers_hut", 78, 85 },
   { "barbarians_gamekeepers_hut", 70, 89 },
   { "barbarians_gamekeepers_hut", 105, 91 },
   { "barbarians_gamekeepers_hut", 119, 80 },
   { "barbarians_gamekeepers_hut", 134, 61 },

   { "barbarians_fortress", 55,  60, soldiers = { [{0,2,0,2}] = 4 }},   -- Must match with soldier level in init
   { "barbarians_sentry",     96,  65, soldiers = { [{0,1,0,2}] = 1 }},   -- close to green
   { "barbarians_sentry",     90,  63, soldiers = { [{0,1,0,2}] = 1 }},   -- close to green
   { "barbarians_fortress", 88,  71, soldiers = { [{0,1,0,2}] = 1 }},
   { "barbarians_barrier",  80,  77, soldiers = { [{0,1,0,2}] = 1 }},   -- dism.
   { "barbarians_barrier",  88,  91, soldiers = { [{0,1,0,2}] = 1 }},
   { "barbarians_barrier",  91,  80, soldiers = { [{0,1,0,2}] = 1 }}, -- dism.
   { "barbarians_fortress", 124,  76, soldiers = { [{0,1,0,2}] = 1 }},
   { "barbarians_fortress", 132,  68, soldiers = { [{0,1,0,2}] = 1 }},
   { "barbarians_fortress", 137,  55, soldiers = { [{0,1,0,2}] = 1 }},
   { "barbarians_sentry",       81,  63, soldiers = { [{0,1,0,2}] = 1 }},   -- beneath lake
   { "barbarians_sentry",       76,  68, soldiers = { [{0,1,0,2}] = 1 }},   -- beneath lake

   { "barbarians_sentry",       75,  98, soldiers = { [{0,1,0,2}] = 1 }},   -- close to brook

   { "barbarians_sentry",      68,  71, soldiers = { [{0,1,0,2}] = 1 }},
   { "barbarians_tower",    66,  80, soldiers = { [{0,1,0,2}] = 1 }},   -- close to border
   { "barbarians_fortress",  72,  87, soldiers = { [{0,1,0,2}] = 1 }},   -- close to border

   { "barbarians_barrier",       82, 103, soldiers = { [{0,1,0,2}] = 1 }},   -- along river
   { "barbarians_barrier",       96,  94, soldiers = { [{0,1,0,2}] = 1 }},
   { "barbarians_barrier",   100,  80, soldiers = { [{0,1,0,2}] = 1 }},
   { "barbarians_barrier",   101,  89, soldiers = { [{0,1,0,2}] = 1 }},
   { "barbarians_sentry",         107,  88, soldiers = { [{0,1,0,2}] = 1 }},
   { "barbarians_barrier",   110,  84, soldiers = { [{0,1,0,2}] = 1 }},
   { "barbarians_barrier",   115,  80, soldiers = { [{0,1,0,2}] = 1 }},

   { "barbarians_sentry",   59,   89, soldiers = { [{0,2,0,2}] = 2 }}      -- in front of the brook
)

-- =======================================================================
                                -- Player 3
-- =======================================================================
p3:forbid_buildings("all")
p3:allow_buildings{
   "barbarians_granitemine",
   "barbarians_coalmine",
   "barbarians_coalmine_deep",
   "barbarians_coalmine_deeper",
   "barbarians_ironmine",
   "barbarians_ironmine_deep",
   "barbarians_ironmine_deeper",
   "barbarians_goldmine",
   "barbarians_goldmine_deep",
   "barbarians_goldmine_deeper",
   "barbarians_quarry",
   "barbarians_lumberjacks_hut",
   "barbarians_rangers_hut",
   "barbarians_fishers_hut",
   "barbarians_hunters_hut",
   "barbarians_gamekeepers_hut",
   "barbarians_well",
   "barbarians_warehouse",
   "barbarians_wood_hardener",
   "barbarians_lime_kiln",
   "barbarians_reed_yard",
   "barbarians_bakery",
   "barbarians_micro_brewery",
   "barbarians_brewery",
   "barbarians_tavern",
   "barbarians_inn",
   "barbarians_big_inn",
   "barbarians_charcoal_kiln",
   "barbarians_smelting_works",
   "barbarians_metal_workshop",
   "barbarians_ax_workshop",
   "barbarians_warmill",
   "barbarians_cattlefarm",
   "barbarians_farm",
   "barbarians_helmsmithy",
   "barbarians_battlearena",
   "barbarians_trainingcamp"
}

-- Room for barbarians_tower after Hjalmar defeated.
p3:conquer(wl.Game().map:get_field(116, 19), 2)

prefilled_buildings(p3,
   { "barbarians_headquarters_interim", 123, 119,
      wares = {
         log = 80,
         blackwood = 32,
         granite = 40,
         grout = 12,
         reed = 24,
         coal = 120,
         iron_ore = 60,
         iron = 12,
         gold_ore = 50,
         gold = 8,
         fish = 6,
         meat = 6,
         barbarians_bread = 8,
         ration = 12,
         snack = 3,
         meal = 4,
         pick = 14,
         shovel = 4,
         felling_ax = 4,
         fishing_rod = 2,
         hunting_spear = 2,
         scythe = 6,
         hammer = 12,
         fire_tongs = 2,
         bread_paddle = 2,
         kitchen_tools = 4,
         ax = 6
      },
      workers = {
         barbarians_carrier = 40,
         barbarians_ox = 20,
         barbarians_stonemason = 2,
         barbarians_lumberjack = 3,
         barbarians_ranger = 1,
         barbarians_fisher = 2,
         barbarians_builder = 10,
         barbarians_lime_burner = 1,
         barbarians_gardener = 1,
         barbarians_brewer = 1,
         barbarians_brewer_master = 1,
         barbarians_blacksmith = 2,
         barbarians_blacksmith_master = 1,
         barbarians_geologist = 6,
         barbarians_miner = 4
      }
   },

   { "barbarians_citadel",             109, 106, soldiers = { [{2,3,0,2}] = 1 }},   -- front with p5
   { "barbarians_citadel",             114, 105, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_citadel",             118, 101, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_citadel",             121,  97, soldiers = { [{2,3,0,2}] = 1 }},

   { "barbarians_citadel",             130, 118, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_citadel",             129, 133, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_fortress",          105, 114, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_fortress",          131, 108, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_fortress",          127,  96, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_barrier",             118, 123, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_citadel",             122, 136, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_citadel",              87, 131, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_fortress",          134,  95, soldiers = { [{2,3,0,2}] = 1 }},

   { "barbarians_barrier",              95, 131, soldiers = { [{2,3,0,2}] = 1 }},   -- around smelting works
   { "barbarians_citadel",             101, 131, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_citadel",             117, 140, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_barrier",             106, 138, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_barrier",             111, 140, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_sentry",             100, 123, soldiers = { [{2,3,0,2}] = 1 }},

   { "barbarians_rangers_hut",       104, 134 },
   { "barbarians_rangers_hut",       112, 138 },

   { "barbarians_helmsmithy",       115, 142, inputs = {iron = 8, gold = 8, coal = 8} },
   { "barbarians_helmsmithy",       100, 128, inputs = {iron = 8, gold = 8, coal = 8} },

   { "barbarians_big_inn",             106, 130, inputs = {fish = 4, meat = 4, barbarians_bread = 4, beer = 4, beer_strong = 4} },
   { "barbarians_big_inn",             108, 130, inputs = {fish = 4, meat = 4, barbarians_bread = 4, beer = 4, beer_strong = 4} },
   { "barbarians_big_inn",             108, 138, inputs = {fish = 4, meat = 4, barbarians_bread = 4, beer = 4, beer_strong = 4} },
   { "barbarians_big_inn",             110, 138, inputs = {fish = 4, meat = 4, barbarians_bread = 4, beer = 4, beer_strong = 4} },

   { "barbarians_smelting_works", 105, 132, inputs = {iron_ore = 8, gold_ore = 8, coal = 8} },
   { "barbarians_smelting_works", 107, 132, inputs = {iron_ore = 8, gold_ore = 8, coal = 8} },
   { "barbarians_smelting_works", 109, 132, inputs = {iron_ore = 8, gold_ore = 8, coal = 8} },
   { "barbarians_smelting_works", 111, 132, inputs = {iron_ore = 8, gold_ore = 8, coal = 8} },

   { "barbarians_warmill",             106, 134, inputs = {iron = 8, gold = 8, coal = 8} },
   { "barbarians_warmill",             108, 134, inputs = {iron = 8, gold = 8, coal = 8} },
   { "barbarians_warmill",             110, 134, inputs = {iron = 8, gold = 8, coal = 8} },
   { "barbarians_warmill",             112, 134, inputs = {iron = 8, gold = 8, coal = 8} },

   { "barbarians_metal_workshop",       107, 136, inputs = {iron = 8, log = 8} },
   { "barbarians_warehouse",          109, 136, workers = {barbarians_carrier = 40, barbarians_ox = 20} },
   { "barbarians_metal_workshop",       111, 136, inputs = {iron = 8, log = 8} },

   { "barbarians_sentry",             110,   3, soldiers = { [{2,3,0,2}] = 1 }},   -- path through mountains
   { "barbarians_citadel",             113,   7, soldiers = { [{2,3,0,2}] = 1 }},
   { "barbarians_barrier",             115,  10, soldiers = { [{2,3,0,2}] = 1 }},

   { "barbarians_warehouse",          129, 101, workers = {barbarians_carrier = 40, barbarians_ox = 20} },
   { "barbarians_warehouse",          114, 113, workers = {barbarians_carrier = 40, barbarians_ox = 20} },
   { "barbarians_warehouse",           94, 124, workers = {barbarians_carrier = 40, barbarians_ox = 20} }
)
