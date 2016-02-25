-- =======================================================================
--                Starting conditions for Headquarters Medium
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

return {
   descname = _ "Headquarters cheat",
   func = function(player, shared_in_start)

   local sf = wl.Game().map.player_slots[player.number].starting_field

   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   prefilled_buildings(player, { "empire_headquarters", sf.x, sf.y,
      wares = {
         armor_helmet = 4,
         spear_wooden = 5,
         felling_ax = 6,
         bread_paddle = 2,
         basket = 2,
         empire_bread = 8,
         cloth = 5,
         coal = 12,
         fire_tongs = 2,
         fish = 6,
         fishing_rod = 2,
         flour = 4,
         gold = 4,
         grape = 4,
         hammer = 14,
         hunting_spear = 2,
         iron = 12,
         iron_ore = 5,
         kitchen_tools = 4,
         marble = 25,
         marble_column = 6,
         meal = 4,
         meat = 6,
         pick = 8,
         ration = 12,
         saw = 2,
         scythe = 5,
         shovel = 6,
         granite = 40,
         log = 30,
         water = 12,
         wheat = 4,
         wine = 8,
         planks = 45,
         wool = 2,
      },
      workers = {
         empire_armorsmith = 1,
         empire_brewer = 1,
         empire_builder = 10,
         empire_carrier = 40,
         empire_charcoal_burner = 1,
         empire_geologist = 4,
         empire_lumberjack = 3,
         empire_miner = 4,
         empire_stonemason = 2,
         empire_toolsmith = 2,
         empire_weaponsmith = 1,
         empire_donkey = 5,
      },
      soldiers = {
         [{0,0,0,0}] = 45,
      }
   })

      place_building_in_region(player, "empire_colosseum", sf:region(11), {
         wares = {
            empire_bread = 8,
            fish = 4,
            meat = 4,
         },
      })

   for i=1,100000 do
     sleep(300000)

     local hq = wl.Game().players[player.number]:get_buildings("empire_headquarters")[1]
     
     if hq and hq.descr.name == "empire_headquarters" then
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
       if hq:get_wares("marble") < 100 then
         hq:set_wares("marble", hq:get_wares("marble") + 5)
       end
       if hq:get_wares("fish") < 50 then
         hq:set_wares("fish", hq:get_wares("fish") + 1)
       end
       if hq:get_wares("gold") < 50 then
         hq:set_wares("gold", hq:get_wares("gold") + 1)
       end
     end
   end
end
}

