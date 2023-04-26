-- =============================================================================
--      Hardcore (Struggling Outpost) starting conditions for Atlanteans
-- =============================================================================

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

   prefilled_buildings(player, { "atlanteans_headquarters", sf.x, sf.y,
      wares = {
         log = 6,
         planks = 6,
         spidercloth = 2,
         granite = 2,
      },
      workers = {
         atlanteans_baker = 1,
         atlanteans_blackroot_farmer = 1,
         atlanteans_builder = 2,
         atlanteans_charcoal_burner = 1,
         atlanteans_carrier = 10,
         atlanteans_farmer = 1,
         atlanteans_fisher = 1,
         atlanteans_forester = 1,
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
         atlanteans_woodcutter = 1,
         atlanteans_weaver = 1,
      },
      soldiers = {
         [{0,0,0,0}] = 2,
      }
   })
end
}

pop_textdomain()
return init
