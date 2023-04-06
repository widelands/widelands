-- =======================================================================
--           Fortified Village starting conditions for Barbarians
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Fortified Village"),
   -- TRANSLATORS: This is the tooltip for the "Fortified Village" starting condition
   tooltip = _("Start the game with a fortified military installation"),
   incompatible_wc = {"scripting/win_conditions/hq_hunter.lua"},

   func =  function(plr, shared_in_start)

      local sf = wl.Game().map.player_slots[plr.number].starting_field

      if shared_in_start then
         sf = shared_in_start
      else
         plr:allow_workers("all")
      end

      local h = plr:place_building("barbarians_citadel", sf, false, true)
      h:set_soldiers{[{0,0,0,0}] = 12}

       place_building_in_region(plr, "barbarians_warehouse", sf:region(7), {
         wares = {
            ax = 5,
            bread_paddle = 2,
            blackwood = 32,
            cloth = 5,
            coal = 1,
            felling_ax = 4,
            fire_tongs = 2,
            fishing_rod = 2,
            grout = 12,
            hammer = 11,
            hunting_spear = 2,
            iron_ore = 5,
            kitchen_tools = 4,
            meal = 4,
            pick = 8,
            ration = 12,
            granite = 34,
            scythe = 6,
            shovel = 4,
            snack = 3,
            reed = 24,
            log = 79,
         },
         workers = {
            barbarians_brewer = 1,
            barbarians_builder = 10,
            barbarians_charcoal_burner = 1,
            barbarians_carrier = 38,
            barbarians_gardener = 1,
            barbarians_geologist = 4,
            barbarians_lumberjack = 2,
            barbarians_miner = 4,
            barbarians_miner_master = 4,
            barbarians_ranger = 1,
            barbarians_stonemason = 2,
            barbarians_trainer = 3,
            barbarians_ox = 5,
         },
         soldiers = {
            [{0,0,0,0}] = 25,
         }
      })

      place_building_in_region(plr, "barbarians_battlearena", sf:region(12), {
         inputs = {
            barbarians_bread = 8,
            fish = 6,
            meat = 6,
         }
      })

      place_building_in_region(plr, "barbarians_trainingcamp", sf:region(12))

      place_building_in_region(plr, "barbarians_helmsmithy", sf:region(12), {
         inputs = { iron = 4, gold = 4 }
      })
      place_building_in_region(plr, "barbarians_metal_workshop", sf:region(12), {
         inputs = { iron = 8 },
      })
      place_building_in_region(plr, "barbarians_ax_workshop", sf:region(12), {
         inputs = { coal = 8 },
      })
      place_building_in_region(plr, "barbarians_wood_hardener", sf:region(12), {
         inputs = { log = 1 },
      })
      place_building_in_region(plr, "barbarians_lime_kiln", sf:region(12), {
         inputs = { granite = 6, coal = 3 },
      })
   end
}
pop_textdomain()
return init
