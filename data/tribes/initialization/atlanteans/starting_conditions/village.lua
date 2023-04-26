-- =======================================================================
--              Village starting conditions for Atlanteans
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Village"),
   -- TRANSLATORS: This is the tooltip for the "Village" starting condition
   tooltip = _("Start the game with a peaceful village"),
   func =  function(plr, shared_in_start)

      local sf = wl.Game().map.player_slots[plr.number].starting_field
      if shared_in_start then
         sf = shared_in_start
      else
         plr:allow_workers("all")
      end

   prefilled_buildings(plr, { "atlanteans_headquarters", sf.x, sf.y,
         wares = {
            diamond = 7,
            iron_ore = 5,
            quartz = 9,
            granite = 50,
            spider_silk = 9,
            log = 16,
            coal = 12,
            gold = 4,
            gold_thread = 6,
            iron = 8,
            planks = 45,
            spidercloth = 5,
            blackroot = 5,
            blackroot_flour = 12,
            atlanteans_bread = 8,
            corn = 5,
            cornmeal = 12,
            fish = 3,
            meat = 3,
            smoked_fish = 6,
            smoked_meat = 6,
            water = 12,
            bread_paddle = 2,
            buckets = 2,
            fire_tongs = 2,
            fishing_net = 4,
            hammer = 11,
            hunting_bow = 1,
            milking_tongs = 2,
            hook_pole = 2,
            pick = 8,
            saw = 9,
            scythe = 4,
            shovel = 9,
            tabard = 5,
            trident_light = 5,
         },
         workers = {
            atlanteans_blackroot_farmer = 1,
            atlanteans_builder = 10,
            atlanteans_charcoal_burner = 1,
            atlanteans_carrier = 38,
            atlanteans_fishbreeder = 1,
            atlanteans_geologist = 4,
            atlanteans_miner = 4,
            atlanteans_stonecutter = 2,
            atlanteans_toolsmith = 1,
            atlanteans_woodcutter = 3,
            atlanteans_horse = 5,
            atlanteans_trainer = 1,
         },
         soldiers = {
            [{0,0,0,0}] = 5,
         },
      })
      place_building_in_region(plr, "atlanteans_weaving_mill", sf:region(10), {
         inputs = {
      spider_silk = 6 ,
      gold_thread = 3
   },
      })

      place_building_in_region(plr, "atlanteans_toolsmithy", sf:region(7), {
         inputs = {log = 5 ,
      spidercloth = 3 ,
      iron = 5
   },
      })

      place_building_in_region(plr, "atlanteans_sawmill", sf:region(7), {
         inputs = { log = 6 }
      })
   end
}
pop_textdomain()
return init
