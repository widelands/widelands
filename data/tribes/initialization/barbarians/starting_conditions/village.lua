-- =======================================================================
--               Village starting conditions for Barbarians
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Village"),
   -- TRANSLATORS: This is the tooltip for the "Village" starting condition
   tooltip = _("Start the game with a peaceful village"),
   func =  function(plr, shared_in_start)

      local sf = wl.Game().map.player_slots[plr.number].starting_field

      if shared_in_start then
         sf = shared_in_start
      else
         plr:allow_workers("all")
      end

   hq = prefilled_buildings(plr, { "barbarians_headquarters", sf.x, sf.y,
         wares = {
         ax = 5,
         bread_paddle = 2,
         blackwood = 32,
         cloth = 5,
         coal = 12,
         felling_ax = 4,
         fire_tongs = 2,
         fish = 6,
         fishing_rod = 2,
         gold = 4,
         grout = 12,
         hammer = 12,
         hunting_spear = 2,
         iron = 8,
         iron_ore = 5,
         kitchen_tools = 4,
         meal = 4,
         meat = 6,
         pick = 8,
         barbarians_bread = 8,
         ration = 12,
         granite = 40,
         scythe = 6,
         shovel = 4,
         snack = 3,
         reed = 24,
         log = 80,
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
            barbarians_ranger = 1,
            barbarians_stonemason = 2,
            barbarians_ox = 5,
            barbarians_trainer = 1,
         },
         soldiers = {
            [{0,0,0,0}] = 5,
         }
      })

      place_building_in_region(plr, "barbarians_metal_workshop", sf:region(8), {
         inputs = { log = 7, iron = 7 },
      })

      place_building_in_region(plr, "barbarians_wood_hardener", sf:region(8), {
         inputs = { log = 5 },
      })
      place_building_in_region(plr, "barbarians_lime_kiln", sf:region(8), {
         inputs = { granite = 5, water = 5, coal = 2 },
      })
   end
}
pop_textdomain()
return init
