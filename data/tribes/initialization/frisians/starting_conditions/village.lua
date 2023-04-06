-- =======================================================================
--              Village starting conditions for Frisians
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Village"),
   -- TRANSLATORS: This is the tooltip for the "Village" starting condition
   tooltip = _("Start the game with a peaceful village"),
   func = function(plr, shared_in_start)

   local sf = wl.Game().map.player_slots[plr.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      plr:allow_workers("all")
   end

   hq = prefilled_buildings(plr, { "frisians_headquarters", sf.x, sf.y,
      wares = {
         log = 60,
         granite = 60,
         reed = 50,
         brick = 50,
         clay = 20,
         water = 10,
         fish = 10,
         meat = 10,
         fruit = 10,
         barley = 5,
         ration = 10,
         honey = 10,
         smoked_meat = 5,
         smoked_fish = 5,
         mead = 5,
         meal = 2,
         coal = 20,
         iron = 5,
         gold = 2,
         iron_ore = 10,
         gold_ore = 2,
         bread_frisians = 10,
         honey_bread = 5,
         beer = 5,
         cloth = 5,
         fur = 10,
         fur_garment = 5,
         sword_short = 5,
         hammer = 10,
         fire_tongs = 2,
         bread_paddle = 2,
         kitchen_tools = 2,
         felling_ax = 3,
         needles = 2,
         basket = 2,
         pick = 5,
         shovel = 5,
         scythe = 3,
         hunting_spear = 2,
         fishing_net = 3,
      },
      workers = {
         frisians_blacksmith = 3,
         frisians_baker = 1,
         frisians_brewer = 1,
         frisians_builder = 10,
         frisians_charcoal_burner = 1,
         frisians_claydigger = 2,
         frisians_brickmaker = 2,
         frisians_carrier = 40,
         frisians_reed_farmer = 2,
         frisians_berry_farmer = 1,
         frisians_farmer = 1,
         frisians_geologist = 4,
         frisians_woodcutter = 3,
         frisians_beekeeper = 1,
         frisians_miner = 4,
         frisians_forester = 2,
         frisians_stonemason = 2,
         frisians_reindeer = 5,
      },
      soldiers = {
         [{0,0,0,0}] = 5,
      }
   })

      place_building_in_region(plr, "frisians_blacksmithy", sf:region(10), {
         inputs = {
            iron = 7,
            log = 7,
            reed = 4
         },
      })

      place_building_in_region(plr, "frisians_charcoal_kiln", sf:region(10), {
         inputs = {
            log = 7,
            clay = 3
         },
      })

      place_building_in_region(plr, "frisians_brick_kiln", sf:region(10), {
         inputs = {
            clay = 2,
            granite = 2,
            coal = 1
         },
      })

end
}
pop_textdomain()
return init
