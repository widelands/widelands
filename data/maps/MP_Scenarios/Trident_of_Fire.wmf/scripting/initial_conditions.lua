function init_human_player(player, f_hq, tribename)
   player:allow_buildings("all")
   if tribename == "barbarians" then
         prefilled_buildings(player,
         {"barbarians_headquarters_interim", f_hq.x, f_hq.y,
            wares = {
                log = 3,
            },
            workers = {
                barbarians_blacksmith = 1,
                barbarians_smelter = 1,
                barbarians_builder = 2,
                barbarians_carrier = 10,
                barbarians_gardener = 1,
                barbarians_geologist = 1,
                barbarians_lime_burner = 1,
                barbarians_lumberjack = 3,
                barbarians_miner = 2,
                barbarians_ranger = 2,
                barbarians_stonemason = 1,
                barbarians_hunter = 1,
                barbarians_farmer = 1,
                barbarians_fisher = 1,
                barbarians_baker = 1,
                barbarians_innkeeper = 1,
                barbarians_charcoal_burner = 1
            },
            soldiers = {
               [{0,0,0,0}] = 3,
            }
         }
      )
   end
   if tribename == "empire" then
      prefilled_buildings(player, { "empire_headquarters_shipwreck", f_hq.x, f_hq.y,
      wares = {
         log = 2,
         planks = 3,
      },
      workers = {
          empire_baker = 1,
          empire_brewer = 2,
          empire_builder = 2,
          empire_carpenter = 1,
          empire_carrier = 10,
          empire_charcoal_burner = 1,
          empire_farmer = 1,
          empire_fisher = 1,
          empire_forester = 2,
          empire_geologist = 1,
          empire_hunter = 1,
          empire_innkeeper = 1,
          empire_lumberjack = 2,
          empire_miller = 1,
          empire_miner = 3,
          empire_smelter = 1,
          empire_stonemason = 2,
          empire_toolsmith = 1,
          empire_vinefarmer =1
        },
        soldiers = {
          [{0,0,0,0}] = 3,
        }
      })

   end
   if tribename == "atlanteans" then
       prefilled_buildings(player, { "atlanteans_headquarters", f_hq.x, f_hq.y,
          wares = {
            log = 2,
            planks = 2,
            spidercloth = 1
         },
         workers = {
             atlanteans_baker = 1,
             atlanteans_blackroot_farmer = 1,
             atlanteans_builder = 2,
             atlanteans_carrier = 10,
             atlanteans_charcoal_burner = 1,
             atlanteans_farmer = 1,
             atlanteans_fisher = 1,
             atlanteans_forester = 2,
             atlanteans_geologist = 1,
             atlanteans_hunter = 1,
             atlanteans_miller = 1,
             atlanteans_miner = 6,
             atlanteans_sawyer = 1,
             atlanteans_smelter = 1,
             atlanteans_smoker = 1,
             atlanteans_spiderbreeder = 1,
             atlanteans_stonecutter = 1,
             atlanteans_toolsmith = 1,
             atlanteans_woodcutter = 2,
             atlanteans_weaver = 1
         },
         soldiers = {
            [{0,0,0,0}] = 3,
         }
       })
   end
   return true
end

function init_AI_player(player, f_hq, f_port, f_vineyard, f_ship, tribename)
   if tribename == "barbarians" then
      player:allow_buildings("all")

      prefilled_buildings(player,
         { "barbarians_port", f_hq.x, f_hq.y,
            wares = {
              ax = 6,
              bread_paddle = 2,
              coal = 12,
              fire_tongs = 2,
              fish = 6,
              grout = 12,
              hammer = 12,
              hunting_spear = 2,
              iron = 12,
              iron_ore = 5,
              kitchen_tools = 4,
              meal = 4,
              meat = 6,
              pick = 14,
              barbarians_bread = 8,
              ration = 12,
              granite = 80,
              scythe = 6,
              shovel = 4,
              snack = 3,
              reed = 24,
              log = 100,
              blackwood = 20
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
         },
         { "barbarians_port", f_port.x, f_port.y}
      )
      player:place_ship(f_ship)
   end

   if tribename == "empire" then
      player:allow_buildings("all")

      prefilled_buildings(player,
         { "empire_port", f_hq.x, f_hq.y,
            wares = {
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
             gold = 4,
             grape = 4,
             hammer = 14,
             hunting_spear = 2,
             iron = 12,
             iron_ore = 5,
             kitchen_tools = 4,
             marble = 60,
             marble_column = 6,
             meal = 4,
             meat = 6,
             pick = 14,
             ration = 12,
             saw = 2,
             scythe = 5,
             shovel = 6,
             granite = 50,
             log = 40,
             water = 12,
             wheat = 4,
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
             empire_miner_master = 1,
             empire_stonemason = 2,
             empire_toolsmith = 2,
             empire_weaponsmith = 1,
             empire_donkey = 5,
            },
            soldiers = { [{0,0,0,0}] = 45 },
         },
         { "empire_port", f_port.x, f_port.y},
         { "empire_vineyard", f_vineyard.x, f_vineyard.y}
      )
      player:place_ship(f_ship)
   end
   if tribename == "atlanteans" then
      player:allow_buildings("all")

      prefilled_buildings(player, { "atlanteans_port", f_hq.x, f_hq.y,
         wares = {
          diamond = 7,
          iron_ore = 5,
          quartz = 9,
          granite = 50,
          spider_silk = 9,
          log = 100,
          coal = 12,
          gold = 4,
          gold_thread = 6,
          iron = 8,
          planks = 50,
          spidercloth = 5,
          blackroot = 5,
          blackroot_flour = 12,
          atlanteans_bread = 8,
          corn = 5,
          cornmeal = 12,
          fish = 3,
          meat = 3,
          smoked_fish = 6,
          smoked_meat = 6,
          water = 12,
          bread_paddle = 2,
          buckets = 2,
          fire_tongs = 2,
          fishing_net = 4,
          hammer = 11,
          hunting_bow = 1,
          milking_tongs = 2,
          hook_pole = 2,
          pick = 12,
          saw = 9,
          scythe = 4,
          shovel = 9,
          tabard = 5,
          trident_light = 5,
         },
         workers = {
          atlanteans_armorsmith = 1,
          atlanteans_blackroot_farmer = 1,
          atlanteans_builder = 10,
          atlanteans_carrier = 40,
          atlanteans_charcoal_burner = 1,
          atlanteans_fishbreeder = 1,
          atlanteans_geologist = 4,
          atlanteans_miner = 4,
          atlanteans_sawyer = 1,
          atlanteans_stonecutter = 2,
          atlanteans_toolsmith = 2,
          atlanteans_weaponsmith = 1,
          atlanteans_woodcutter = 3,
          atlanteans_horse = 5,
         },
         soldiers = { [{0,0,0,0}] = 45 },
         },
         { "atlanteans_port", f_port.x, f_port.y}
      )
      player:place_ship(f_ship)
   end
   return true
end
