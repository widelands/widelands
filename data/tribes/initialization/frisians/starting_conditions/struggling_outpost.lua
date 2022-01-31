-- =======================================================================
--    Hardcore (Struggling Outpost) starting conditions for Frisians
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Struggling Outpost"),
   -- TRANSLATORS: This is the tooltip for the "Hardcore" starting condition
   tooltip = _("Start the game with just your headquarters and very few wares for bootstrapping an economy. Warning: the AI can’t successfully start from this."),
   func =  function(player, shared_in_start)

   local sf = wl.Game().map.player_slots[player.number].starting_field

   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   prefilled_buildings(player, { "frisians_headquarters", sf.x, sf.y,
      wares = {
         reed = 4,
         log = 3,
         brick = 14,
         coal = 3,
         granite = 2,
      },
      workers = {
         frisians_baker = 1,
         frisians_fruit_collector = 1,
         frisians_builder = 2,
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
         frisians_miner = 2,
         frisians_smelter = 1,
         frisians_stonemason = 1,
         frisians_blacksmith = 1,
         frisians_reed_farmer = 1,
         frisians_berry_farmer = 1
      },
      soldiers = {
         [{0,0,0,0}] = 2,
      }
   })
end
}
pop_textdomain()
return init
