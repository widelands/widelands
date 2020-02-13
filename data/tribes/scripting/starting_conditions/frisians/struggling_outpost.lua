-- =======================================================================
--                Hardcore Starting conditions for frisians
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

   prefilled_buildings(player, { "frisians_headquarters", sf.x, sf.y,
      wares = {
         reed = 2,
         log = 1,
         brick = 9,
         coal = 1,
      },
      workers = {
         frisians_baker = 1,
         frisians_brewer = 1,
         frisians_builder = 1,
         frisians_brickmaker = 1,
         frisians_carrier = 10,
         frisians_charcoal_burner = 1,
         frisians_farmer = 1,
         frisians_fisher = 1,
         frisians_forester = 1,
         frisians_geologist = 1,
         frisians_hunter = 1,
         frisians_landlady = 1,
         frisians_woodcutter = 1,
         frisians_claydigger = 1,
         frisians_miner = 3,
         frisians_smelter = 1,
         frisians_stonemason = 1,
         frisians_blacksmith = 1,
         frisians_reed_farmer = 1,
         frisians_berry_farmer = 1
      },
      soldiers = {
         [{0,0,0,0}] = 1
      }
   })
end
}
