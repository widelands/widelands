-- =======================================================================
--                                 Player 1
-- =======================================================================
-- p1.see_all = true
p1:forbid_buildings("all")
p1:allow_buildings{
   "empire_outpost",
   "empire_sentry",
   "empire_blockhouse",
   "empire_lumberjacks_house",
   "empire_quarry",
}
hq = prefilled_buildings(p1,
   {"empire_headquarters_shipwreck", 4, 6,
      wares = {
         basket = 1,
         empire_bread = 8,
         bread_paddle = 2,
         cloth = 5,
         coal = 12,
         felling_ax = 6,
         fire_tongs = 2,
         fish = 6,
         fishing_rod = 3,
         flour = 6,
         gold = 2,
         granite = 20,
         grape = 4,
         hammer = 12,
         armor_helmet = 4,
         hunting_spear = 2,
         iron = 12,
         iron_ore = 5,
         kitchen_tools = 4,
         log = 30,
         marble = 25,
         marble_column = 6,
         meal = 4,
         meat = 6,
         pick = 14,
         planks = 45,
         ration = 12,
         saw = 3,
         scythe = 5,
         shovel = 6,
         spear_wooden = 5,
         water = 12,
         wheat = 6,
         wine = 8,
         wool = 2,
      },
      workers = {
         empire_armorsmith = 1,
         empire_brewer = 1,
         empire_builder = 10,
         empire_carrier = 40,
         empire_charcoal_burner = 1,
         empire_donkey = 20,
         empire_geologist = 4,
         empire_lumberjack = 3,
         empire_miner = 4,
         empire_stonemason = 2,
         empire_toolsmith = 2,
         empire_weaponsmith = 1,
      },
      soldiers = {
         [{0,0,0,0}] = 45,
      }
   }
)

-- =======================================================================
--                                 Player 2
-- =======================================================================
p2:forbid_buildings("all")
p2:allow_buildings{
   "barbarians_bakery",
   "barbarians_barrier",
   "barbarians_farm",
   "barbarians_fishers_hut",
   "barbarians_hunters_hut",
   "barbarians_lime_kiln",
   "barbarians_lumberjacks_hut",
   "barbarians_micro_brewery",
   "barbarians_rangers_hut",
   "barbarians_reed_yard",
   "barbarians_sentry",
   "barbarians_tavern",
   "barbarians_well",
   "barbarians_wood_hardener",
   "barbarians_quarry",
   "barbarians_gamekeepers_hut",
}

prefilled_buildings(p2,
   {"barbarians_headquarters", 34, 74,
   wares = {
      ax = 6,
      blackwood = 32,
      barbarians_bread = 8,
      bread_paddle = 2,
      cloth = 5,
      coal = 12,
      fire_tongs = 2,
      fish = 6,
      fishing_rod = 2,
      gold = 4,
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
   },
   workers = {
      barbarians_blacksmith = 2,
      barbarians_brewer = 1,
      barbarians_builder = 10,
      barbarians_carrier = 40,
      barbarians_charcoal_burner = 1,
      barbarians_gardener = 1,
      barbarians_geologist = 4,
      barbarians_lime_burner = 1,
      barbarians_lumberjack = 3,
      barbarians_miner = 4,
      barbarians_ranger = 1,
      barbarians_stonemason = 2,
   },

    soldiers = {
       [{0,0,0,0}] = 45,
    }
  }
)
