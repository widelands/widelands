-- =======================================================================
--                                 Player 1                                 
-- =======================================================================
p1:allow_buildings("all")

prefilled_buildings(p1, { "headquarters_interim", 65, 6, 
   wares = {
      ax = 6,
      bread_paddle = 2,
      blackwood = 32,
      coal = 12,
      fire_tongs = 2,
      fish = 6,
      grout = 12,
      hammer = 12,
      hunting_spear = 2,
      iron = 12,
      ironore = 5,
      kitchen_tools = 4,
      meal = 4,
      meat = 6,
      pick = 14,
      pittabread = 8,
      ration = 12,
      raw_stone = 40,
      scythe = 6,
      shovel = 4,
      snack = 3,
      thatchreed = 24,
      log = 80,
   },
   workers = {
      carrier = 40,
      blacksmith = 2,
      brewer = 1,
      builder = 10,
      gardener = 1,
      fisher = 2,
      geologist = 4,
      ["lime-burner"] = 1,
      lumberjack = 3,
      miner = 4,
      ranger = 1,
      stonemason = 2,
      ox = 20, 
   },
   soldiers = { [{0,0,0,0}] = 45 },
})

-- =======================================================================
--                                 Player 2                                 
-- =======================================================================
p2:forbid_buildings("all")
p2:allow_buildings{ "lumberjacks_hut" }

prefilled_buildings(p2,
   { "warehouse", 45, 40, wares = {log = 20,}, workers = {
      carrier = 40,
      builder = 2,
      lumberjack = 10,
   }},
   { "donjon", 14, 67, soldiers = {
      [{0,0,0,0}] = 1,
      [{2,0,0,0}] = 1,
      [{3,1,0,0}] = 1,
   }},
   { "donjon", 31, 65, soldiers = {
      [{2,1,0,0}] = 2,
      [{1,0,0,0}] = 1,
   }},
   { "donjon", 53, 17, soldiers = {
      [{0,0,0,0}] = 2,
      [{2,1,0,0}] = 1,
   }},
   { "barrier", 40, 24, soldiers = {
      [{2,0,0,0}] = 2,
      [{0,0,0,1}] = 1,
   }},
   { "barrier", 39, 53, soldiers = {
      [{2,0,0,0}] = 4,
   }},
   { "barrier", 50, 44, soldiers = {
      [{0,0,0,0}] = 2,
      [{0,1,0,1}] = 1,
      [{3,1,0,1}] = 1,
   }},
   { "barrier", 26, 75, soldiers = {
      [{3,1,0,0}] = 3,
      [{0,0,0,0}] = 1,
   }},
   { "barrier", 43, 44, soldiers = {
      [{0,0,0,0}] = 2,
   }},
   { "barrier", 20, 71, soldiers = {
      [{0,1,0,1}] = 1,
      [{1,0,0,0}] = 1,
      [{0,0,0,0}] = 1,
   }},
   { "barrier", 50, 30, soldiers = {
      [{0,0,0,0}] = 1,
   }},
   { "sentry", 43, 29, soldiers = {
      [{0,0,0,0}] = 2,
   }},
   { "sentry", 42, 9, soldiers = {
      [{0,0,0,0}] = 1,
   }},
   { "sentry", 30, 76, soldiers = {
      [{0,0,0,0}] = 2,
   }},
   { "citadel", 47, 13, soldiers = {
      [{0,0,0,0}] = 2,
      [{1,0,0,0}] = 1,
      [{1,1,0,0}] = 1,
      [{2,0,0,0}] = 2,
      [{3,1,0,0}] = 3,
   }},
   { "fortress", 42, 72, soldiers = {
      [{0,0,0,0}] = 2,
      [{1,0,0,0}] = 1,
      [{1,1,0,0}] = 1,
      [{2,0,0,0}] = 2,
      [{3,1,0,0}] = 1,
   }},
   { "high_tower.atlanteans", 52, 26, soldiers = {
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
   "lumberjacks_hut",
   "burners_house",
   "quarry",
   "rangers_hut",
   "hardener",
   "lime_kiln",
   "reed_yard",
   "fishers_hut",
   "well",
   "tavern",
   "hunters_hut",
   "micro-brewery",
   "farm",
   "barrier",
   "donjon",
   "sentry",
}

prefilled_buildings(p3, 
   { "warehouse", 6, 44, wares = {
         log = 200,
         kitchen_tools = 4,
         pick = 14,
         raw_stone = 40,
         scythe = 6,
         shovel = 4,
         thatchreed = 24,
         coal = 40,
      }, 
      workers = {
         carrier = 40,
         builder = 4,
         lumberjack = 10,
         burner = 1,
         brewer = 1,
         farmer = 1,
         gardener = 1,
         fisher = 2,
         geologist = 4,
         ["lime-burner"] = 1,
         ranger = 1,
         hunter = 1,
         gamekeeper = 1,
      },
      soldiers = { [{0,0,0,0}] = 30 },
   },
   { "barrier", 6, 51, soldiers = {
      [{0,0,0,0}] = 2,
      [{0,0,0,1}] = 1,
   }},
   { "tower.atlanteans", 79, 50, soldiers = {
      [{0,0,0,0}] = 2,
      [{0,0,0,1}] = 1,
   }},
   { "guardhouse.atlanteans", 7, 42, soldiers = {
      [{0,0,0,0}] = 2,
   }},
   { "farm",  8, 48 },
   { "farm",  4, 52 },
   { "farm", 10, 49 }
)

-- =======================================================================
--                                 Player 4                                 
-- =======================================================================
p4:forbid_buildings("all")
prefilled_buildings(p4, 
   { "castle.atlanteans", 23, 19, soldiers = {
      [{0,0,0,0}] = 2,
      [{2,0,0,0}] = 2,
      [{0,1,0,0}] = 2,
      [{2,1,0,0}] = 2,
      [{1,1,0,0}] = 1,
   }}
)


