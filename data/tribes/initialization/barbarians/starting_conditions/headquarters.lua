-- =======================================================================
--             Headquarters starting conditions for Barbarians
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Headquarters"),
   -- TRANSLATORS: This is the tooltip for the "Headquarters" starting condition
   tooltip = _("Start the game with your headquarters only"),
   func = function(player, shared_in_start)

   local sf = wl.Game().map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   hq = prefilled_buildings(player, { "barbarians_headquarters", sf.x, sf.y,
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
         iron = 12,
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
         barbarians_blacksmith = 2,
         barbarians_brewer = 1,
         barbarians_builder = 10,
         barbarians_charcoal_burner = 1,
         barbarians_carrier = 40,
         barbarians_gardener = 1,
         barbarians_geologist = 4,
         barbarians_lime_burner = 1,
         barbarians_lumberjack = 3,
         barbarians_miner = 4,
         barbarians_miner_master = 4,
         barbarians_ranger = 1,
         barbarians_stonemason = 2,
         barbarians_trainer = 3,
         barbarians_ox = 5,
      },
      soldiers = {
         [{0,0,0,0}] = 45,
      }
   })
end
}

pop_textdomain()
return init
