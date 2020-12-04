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
        water = 128,
        log = 64,
        planks = 32,
        spidercloth = 32,
        spider_silk = 32,
        reed = 32,
        granite = 64,
        grout = 32,
        brick = 32,
        marble = 16,
        diamond = 16,
        quartz = 16,
        ore = 16,
        coal = 64,
        hammer = 10,
        saw = 10,
        shovel = 10,
        buckets = 10,
        felling_ax = 8,
        pick = 8,
        fire_tongs = 2,
        scythe = 4,
        basket = 4,
      },
      workers = {
         europeans_carrier = 32,
         europeans_builder = 8,
         europeans_geologist = 2,
         europeans_trainer = 4,
      },
      soldiers = {
         [{0,0,0,0}] = 32,
      }
   })
end
}

return init
