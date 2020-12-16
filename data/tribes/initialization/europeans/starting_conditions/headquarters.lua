-- =======================================================================
--              Headquarters Starting Conditions for Europeans
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

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
            water = 1024,
            log = 512,
            granite = 64,
            coal = 64,
            planks = 48,
            reed = 48,
            iron = 32,
            spider_silk = 32,
            spidercloth = 32,
            grout = 32,
            brick = 32,
            marble = 32,
            quartz = 32,
            diamond = 16,
        },
        workers = {
            europeans_carrier = 32,
            europeans_farmer_basic = 16,
            europeans_builder = 10,
            europeans_miner_basic = 6,
            europeans_trainer = 4,
            europeans_lumberjack_basic = 4,
            europeans_forester_basic = 4,
            europeans_carpenter_basic = 2,
            europeans_stonecutter_basic = 2,
            europeans_stonemason_basic = 2,
            europeans_charcoal_burner_basic = 2,
            europeans_hunter_basic = 2,
            europeans_fisher_basic = 2,
            europeans_fishbreeder = 2,
            europeans_miller_basic = 2,
            europeans_baker_basic = 2,
            europeans_smoker_basic = 2,
            europeans_brewer_basic = 2,
            europeans_breeder_normal = 2,
            europeans_weaver_basic = 2,
            europeans_smelter_basic = 2,
            europeans_smith_basic = 2,
            europeans_shipwright = 2,
            europeans_geologist = 1
      },
        soldiers = {
            [{0,0,0,0}] = 32,
      }
   })
end
}
pop_textdomain()
return init
