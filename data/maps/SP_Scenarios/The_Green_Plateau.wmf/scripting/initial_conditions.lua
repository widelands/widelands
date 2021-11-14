-- =======================================================================
--                                 Player 1
-- =======================================================================
p1:allow_buildings("all")

prefilled_buildings(p1, { "barbarians_headquarters_interim", 65, 6,
   wares = {
      blackwood = 32,
      barbarians_bread = 8,
      bread_paddle = 2,
      coal = 12,
      fire_tongs = 2,
      fish = 6,
      granite = 40,
      grout = 12,
      hammer = 12,
      hunting_spear = 2,
      iron = 12,
      iron_ore = 5,
      kitchen_tools = 4,
      log = 80,
      meal = 4,
      meat = 6,
      pick = 14,
      ration = 12,
      scythe = 6,
      shovel = 4,
      snack = 3,
      reed = 24,
     ax = 6,
   },
   workers = {
      barbarians_carrier = 40,
      barbarians_blacksmith = 2,
      barbarians_brewer = 1,
      barbarians_builder = 10,
      barbarians_gardener = 1,
      barbarians_fisher = 2,
      barbarians_geologist = 4,
      barbarians_lime_burner = 1,
      barbarians_lumberjack = 3,
      barbarians_miner = 4,
      barbarians_ranger = 1,
      barbarians_stonemason = 2,
      barbarians_ox = 20,
   },
   soldiers = { [{0,0,0,0}] = 45 },
})

-- =======================================================================
--                                 Player 2
-- =======================================================================
p2:forbid_buildings("all")
p2:allow_buildings{ "barbarians_lumberjacks_hut" }

prefilled_buildings(p2,
   { "barbarians_warehouse", 45, 40, wares = {log = 20,}, workers = {
      barbarians_carrier = 40,
      barbarians_builder = 2,
      barbarians_lumberjack = 10,
   }},
   { "barbarians_tower", 14, 67, soldiers = {
      [{0,0,0,0}] = 1,
      [{2,0,0,0}] = 1,
      [{3,1,0,0}] = 1,
   }},
   { "barbarians_tower", 31, 65, soldiers = {
      [{2,1,0,0}] = 2,
      [{1,0,0,0}] = 1,
   }},
   { "barbarians_tower", 53, 17, soldiers = {
      [{0,0,0,0}] = 2,
      [{2,1,0,0}] = 1,
   }},
   { "barbarians_barrier", 40, 24, soldiers = {
      [{2,0,0,0}] = 2,
      [{0,0,0,1}] = 1,
   }},
   { "barbarians_barrier", 39, 53, soldiers = {
      [{2,0,0,0}] = 4,
   }},
   { "barbarians_barrier", 50, 44, soldiers = {
      [{0,0,0,0}] = 2,
      [{0,1,0,1}] = 1,
      [{3,1,0,1}] = 1,
   }},
   { "barbarians_barrier", 26, 75, soldiers = {
      [{3,1,0,0}] = 3,
      [{0,0,0,0}] = 1,
   }},
   { "barbarians_barrier", 43, 44, soldiers = {
      [{0,0,0,0}] = 2,
   }},
   { "barbarians_barrier", 20, 71, soldiers = {
      [{0,1,0,1}] = 1,
      [{1,0,0,0}] = 1,
      [{0,0,0,0}] = 1,
   }},
   { "barbarians_barrier", 50, 30, soldiers = {
      [{0,0,0,0}] = 1,
   }},
   { "barbarians_sentry", 43, 29, soldiers = {
      [{0,0,0,0}] = 2,
   }},
   { "barbarians_sentry", 42, 9, soldiers = {
      [{0,0,0,0}] = 1,
   }},
   { "barbarians_sentry", 30, 76, soldiers = {
      [{0,0,0,0}] = 2,
   }},
   { "barbarians_citadel", 47, 13, soldiers = {
      [{0,0,0,0}] = 2,
      [{1,0,0,0}] = 1,
      [{1,1,0,0}] = 1,
      [{2,0,0,0}] = 2,
      [{3,1,0,0}] = 3,
   }},
   { "barbarians_fortress", 42, 72, soldiers = {
      [{0,0,0,0}] = 2,
      [{1,0,0,0}] = 1,
      [{1,1,0,0}] = 1,
      [{2,0,0,0}] = 2,
      [{3,1,0,0}] = 1,
   }},
   { "atlanteans_tower_high", 52, 26, soldiers = {
      [{1,1,0,0}] = 1,
      [{2,0,0,0}] = 2,
      [{3,1,0,0}] = 1,
   }}
)
-- =======================================================================
--                                 Player 3
-- =======================================================================
p3:forbid_buildings("all")
p3:allow_buildings{
   "barbarians_lumberjacks_hut",
   "barbarians_charcoal_kiln",
   "barbarians_quarry",
   "barbarians_rangers_hut",
   "barbarians_wood_hardener",
   "barbarians_lime_kiln",
   "barbarians_reed_yard",
   "barbarians_fishers_hut",
   "barbarians_well",
   "barbarians_tavern",
   "barbarians_hunters_hut",
   "barbarians_micro_brewery",
   "barbarians_farm",
   "barbarians_barrier",
   "barbarians_tower",
   "barbarians_sentry",
}

prefilled_buildings(p3,
   { "barbarians_warehouse", 6, 44, wares = {
         coal = 40,
         granite = 40,
         kitchen_tools = 4,
         pick = 14,
         scythe = 6,
         shovel = 4,
         reed = 24,
         log = 200,
      },
      workers = {
         barbarians_carrier = 40,
         barbarians_builder = 4,
         barbarians_lumberjack = 10,
         barbarians_charcoal_burner = 1,
         barbarians_brewer = 1,
         barbarians_farmer = 1,
         barbarians_gardener = 1,
         barbarians_fisher = 2,
         barbarians_geologist = 4,
         barbarians_lime_burner = 1,
         barbarians_ranger = 1,
         barbarians_hunter = 1,
         barbarians_gamekeeper = 1,
      },
      soldiers = { [{0,0,0,0}] = 30 },
   },
   { "barbarians_barrier", 6, 51, soldiers = {
      [{0,0,0,0}] = 2,
      [{0,0,0,1}] = 1,
   }},
   { "atlanteans_tower", 79, 50, soldiers = {
      [{0,0,0,0}] = 2,
      [{0,0,0,1}] = 1,
   }},
   { "atlanteans_guardhouse", 7, 42, soldiers = {
      [{0,0,0,0}] = 2,
   }},
   { "barbarians_farm",  8, 48 },
   { "barbarians_farm",  4, 52 },
   { "barbarians_farm", 10, 49 }
)

-- =======================================================================
--                                 Player 4
-- =======================================================================
p4:forbid_buildings("all")
prefilled_buildings(p4,
   { "atlanteans_castle", 23, 19, soldiers = {
      [{0,0,0,0}] = 2,
      [{2,0,0,0}] = 2,
      [{0,1,0,0}] = 2,
      [{2,1,0,0}] = 2,
      [{1,1,0,0}] = 1,
   }}
)
