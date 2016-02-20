-- =======================================================================
--              Headquarters Starting Conditions for Atlanteans
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

init = {
   descname = _ "Headquarters cheat",
   func =  function(player, shared_in_start)

   local sf = wl.Game().map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   prefilled_buildings(player, { "atlanteans_headquarters", sf.x, sf.y,
      wares = {
         diamond = 7,
         iron_ore = 5,
         quartz = 9,
         granite = 50,
         spider_silk = 9,
         log = 20,
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
         atlanteans_armorsmith = 1,
         atlanteans_blackroot_farmer = 1,
         atlanteans_builder = 10,
         atlanteans_charcoal_burner = 1,
         atlanteans_carrier = 40,
         atlanteans_fishbreeder = 1,
         atlanteans_geologist = 4,
         atlanteans_miner = 4,
         atlanteans_sawyer = 1,
         atlanteans_stonecutter = 2,
         atlanteans_toolsmith = 2,
         atlanteans_weaponsmith = 1,
         atlanteans_woodcutter = 3,
         atlanteans_horse = 5,
      },
      soldiers = {
         [{0,0,0,0}] = 35,
      }
   })

      place_building_in_region(player, "atlanteans_labyrinth", sf:region(11), {
         wares = {
            atlanteans_bread = 4,
            smoked_fish = 3,
            smoked_meat = 3,
         }
      })

   for i=1,100000 do
     sleep(300000)

     local hq = wl.Game().players[player.number]:get_buildings("atlanteans_headquarters")[1]

     if hq and hq.descr.name == "atlanteans_headquarters" then
       if hq:get_wares("water") < 100 then
         hq:set_wares("water", hq:get_wares("water") + 20)
       end
       if hq:get_wares("log") < 100 then
         hq:set_wares("log", hq:get_wares("log") + 20)
       end
       if hq:get_wares("granite") < 100 then
         hq:set_wares("granite", hq:get_wares("granite") + 10)
       end
       if hq:get_wares("coal") < 100 then
         hq:set_wares("coal", hq:get_wares("coal") + 5)
       end
       if hq:get_wares("iron_ore") < 100 then
         hq:set_wares("iron_ore", hq:get_wares("iron_ore") + 5)
       end
       if hq:get_wares("quartz") < 10 then
         hq:set_wares("quartz", hq:get_wares("quartz") + 1)
       end
       if hq:get_wares("meat") < 50 then
         hq:set_wares("meat", hq:get_wares("meat") + 1)
       end
       if hq:get_wares("gold") < 10 then
         hq:set_wares("gold", hq:get_wares("gold") + 1)
       end
     end
   end
end
}

return init

