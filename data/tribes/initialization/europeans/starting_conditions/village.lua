-- =======================================================================
--              Village Starting Conditions for Europeans
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "Village",
   -- TRANSLATORS: This is the tooltip for the "Headquarters" starting condition
   tooltip = _"Start the game with a small peaceful village",
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
            planks = 48,
            reed = 48,
            corn = 32,
            coal = 32,
            marble = 16,
            quartz = 16,
            diamond = 16,
            iron = 16,
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
   place_building_in_region(plr, "europeans_weaving_mill_basic", sf:region(10), {
        inputs = {spider_silk = 8},
   })
   place_building_in_region(plr, "europeans_spiderfarm", sf:region(10), {
        inputs = {water = 6, corn = 6},
   })
   place_building_in_region(plr, "europeans_reed_yard", sf:region(10), {
        inputs = {water = 6},
   })
   place_building_in_region(plr, "europeans_sawmill_basic", sf:region(8), {
        inputs = {log = 8},
   })
   place_building_in_region(plr, "europeans_charcoal_kiln_basic", sf:region(8), {
        inputs = {log = 12},
   })
   place_building_in_region(plr, "europeans_lime_kiln", sf:region(8), {
        inputs = {granite = 8, clay = 6, water = 6, coal = 6},
   })
   place_building_in_region(plr, "europeans_smelting_works_basic", sf:region(8), {
        inputs = {coal = 8, ore = 8},
   })
   place_building_in_region(plr, "europeans_smithy_basic", sf:region(6), {
        inputs = {planks = 4, iron = 4},
   })
   place_building_in_region(plr, "europeans_well_normal", sf:region(6), {
   })

   
end
}
pop_textdomain()
return init
