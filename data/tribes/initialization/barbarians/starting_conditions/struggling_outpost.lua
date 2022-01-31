-- =======================================================================
--    Hardcore (Struggling Outpost) starting conditions for Barbarians
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Struggling Outpost"),
   -- TRANSLATORS: This is the tooltip for the "Hardcore" starting condition
   tooltip = _("Start the game with just your headquarters and very few wares for bootstrapping an economy. Warning: the AI can’t successfully start from this."),
   func = function(player, shared_in_start)

   local sf = wl.Game().map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   hq = prefilled_buildings(player, { "barbarians_headquarters", sf.x, sf.y,
      wares = {
         log = 10,
         granite = 2,
      },
      workers = {
         barbarians_blacksmith = 1,
         barbarians_smelter = 1,
         barbarians_builder = 2,
         barbarians_carrier = 10,
         barbarians_gardener = 1,
         barbarians_geologist = 1,
         barbarians_lime_burner = 1,
         barbarians_lumberjack = 2,
         barbarians_miner = 2,
         barbarians_ranger = 1,
         barbarians_stonemason = 1,
         barbarians_hunter = 1,
         barbarians_fisher = 1,
         barbarians_innkeeper = 1,
         barbarians_charcoal_burner = 1
      },
      soldiers = {
         [{0,0,0,0}] = 2,
      }
   })
end
}

pop_textdomain()
return init
