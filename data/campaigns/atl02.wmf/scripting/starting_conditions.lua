-- =======================================================================
--                      Atlanteans Tutorial Mission 02
-- =======================================================================

-- ===============
-- Initialization
-- ===============

-- Initializing p1 (the scenario player)
   p1:allow_buildings("all")
   p1:allow_workers("all")

   p1:forbid_buildings{"atlanteans_shipyard"}

   local sf = map.player_slots[1].starting_field
   prefilled_buildings(p1, { "atlanteans_headquarters", sf.x, sf.y,
      wares = {
         diamond = 2,
         iron_ore = 50,
         quartz = 3,
         granite = 8,
         spider_silk = 4,
         log = 40,
         coal = 150,
         gold = 0,
         gold_thread = 0,
         iron = 0,
         planks = 23,
         spidercloth = 5,
         blackroot = 0,
         blackroot_flour = 20,
         atlanteans_bread = 10,
         corn = 0,
         cornmeal = 20,
         fish = 0,
         meat = 0,
         smoked_fish = 16,
         smoked_meat = 17,
         water = 4,
         bread_paddle = 0,
         buckets = 0,
         fire_tongs = 0,
         fishing_net = 0,
         hammer = 0,
         hunting_bow = 0,
         milking_tongs = 0,
         hook_pole = 0,
         pick = 0,
         saw = 0,
         scythe = 0,
         shovel = 0,
         tabard = 0,
         trident_light = 0,
      },
      workers = {
         atlanteans_armorsmith = 0,
         atlanteans_blackroot_farmer = 0,
         atlanteans_builder = 10,
         atlanteans_charcoal_burner = 0,
         atlanteans_carrier = 40,
         atlanteans_fishbreeder = 1,
         atlanteans_geologist = 4,
         atlanteans_miner = 2,
         atlanteans_sawyer = 1,
         atlanteans_smelter = 1,
         atlanteans_stonecutter = 1,
         atlanteans_toolsmith = 1,
         atlanteans_weaponsmith = 0,
         atlanteans_woodcutter = 3,
         atlanteans_horse = 5,
      },
      soldiers = {
         [{0,0,0,0}] = 10,
      }
   })

-- Initialize Maletus (Player 2)
   Maletus:allow_buildings("all")

   sf = map.player_slots[2].starting_field
   Maletus:place_flag(sf.brn,true)
   create_economy_for_plr_2()

-- Initialize Kalitath (Player 3)
   Kalitath:allow_buildings("all")

   sf = map.player_slots[3].starting_field
   Kalitath:place_flag(sf.brn,true)
   create_economy_for_plr_3()