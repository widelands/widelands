-- =======================================================================
--              Headquarters Starting Conditions for Europeans
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "Headquarters",
   -- TRANSLATORS: This is the tooltip for the "Headquarters" starting condition
   tooltip = _"Start the game with your headquarters only",
   func =  function(plr, shared_in_start)

   local sf = wl.Game().map.player_slots[plr.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      plr:allow_workers("all")
   end

   prefilled_buildings(plr, { "europeans_headquarters", sf.x, sf.y,
      wares = {
      water = 128,
      log = 64,
      planks = 40,
      spidercloth = 40,
      granite = 64,
      reed = 32,
      grout = 32,
      brick = 32,
      marble = 16,
      diamond = 8,
      quartz = 8,
         ore = 16,
         coal = 64,
         basket = 10,
         hammer = 10,
         saw = 10,
         shovel = 9,
         pick = 8,
         fire_tongs = 2,
         buckets = 4,
         scythe = 4,
      },
      workers = {
         europeans_builder = 10,
         europeans_carrier = 40,
         europeans_geologist = 4,
         europeans_miner = 4,
         europeans_stonecutter = 2,
         europeans_stonemason = 2,
         europeans_toolsmith = 2,
         europeans_trainer = 3,
            europeans_lumberjack = 3,
            europeans_woodcutter = 3,
            europeans_ranger = 3,
            europeans_forester = 3,
            europeans_sawyer = 2,
            europeans_fisher = 2,
            europeans_hunter = 2,
            europeans_smoker = 1,
            europeans_miller = 1,
            europeans_baker = 1,
            europeans_spiderbreeder = 1,
            europeans_weaver = 1,
      },
      soldiers = {
         [{0,0,0,0}] = 35,
      }
   })
end
}

return init
