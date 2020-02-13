-- =======================================================================
--                Hardcore Starting conditions for Empire
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

return {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "Struggling Outpost",
   -- TRANSLATORS: This is the tooltip for the "Hardcore" starting condition
   tooltip = _"Start the game with just your headquarters and a minimum for bootstrapping an economy. Warning: the AI can't successfully start from this",
   func =  function(player, shared_in_start)

   local sf = wl.Game().map.player_slots[player.number].starting_field

   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   prefilled_buildings(player, { "empire_headquarters", sf.x, sf.y,
      wares = {
         log = 2,
         planks = 3
      },
      workers = {
         empire_baker = 1,
         empire_brewer = 1,
         empire_builder = 1,
         empire_carpenter = 1,
         empire_carrier = 10,
         empire_charcoal_burner = 1,
         empire_farmer = 1,
         empire_fisher = 1,
         empire_forester = 1,
         empire_geologist = 1,
         empire_hunter = 1,
         empire_innkeeper = 1,
         empire_lumberjack = 1,
         empire_miller = 1,
         empire_miner = 3,
         empire_smelter = 1,
         empire_stonemason = 2,
         empire_toolsmith = 1,
         empire_vinefarmer = 1,
         empire_vintner = 1
      },
      soldiers = {
         [{0,0,0,0}] = 1
      }
   })
end
}
