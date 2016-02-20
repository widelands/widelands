-- =======================================================================
--                 Start conditions for Headquarters Medium
-- =======================================================================

include "scripting/infrastructure.lua"
include "scripting/coroutine.lua"

set_textdomain("tribes")

init = {
   descname = _ "Headquarters cheat",
   func = function(player, shared_in_start)

   local sf = wl.Game().map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   hq = prefilled_buildings(player, { "barbarians_headquarters", sf.x, sf.y,
      wares = {
         ax = 5,
         bread_paddle = 2,
         blackwood = 32,
         cloth = 5,
         coal = 12,
         felling_ax = 4,
         fire_tongs = 2,
         fish = 6,
         fishing_rod = 2,
         gold = 4,
         grout = 12,
         hammer = 12,
         hunting_spear = 2,
         iron = 12,
         iron_ore = 5,
         kitchen_tools = 4,
         meal = 4,
         meat = 6,
         pick = 8,
         barbarians_bread = 8,
         ration = 12,
         granite = 40,
         scythe = 6,
         shovel = 4,
         snack = 3,
         thatch_reed = 24,
         log = 80,
      },
      workers = {
         barbarians_blacksmith = 2,
         barbarians_brewer = 1,
         barbarians_builder = 10,
         barbarians_charcoal_burner = 1,
         barbarians_carrier = 40,
         barbarians_gardener = 1,
         barbarians_geologist = 4,
         barbarians_lime_burner = 1,
         barbarians_lumberjack = 3,
         barbarians_miner = 4,
         barbarians_ranger = 1,
         barbarians_stonemason = 2,
         barbarians_ox = 5,
      },
      soldiers = {
         [{0,0,0,0}] = 45,
      }
   })

      place_building_in_region(player, "barbarians_battlearena", sf:region(12), {
         wares = {
            barbarians_bread = 8,
            fish = 6,
            meat = 6,
         }
      })

   for i=1,100000 do
     sleep(300000)

     local hq = wl.Game().players[player.number]:get_buildings("barbarians_headquarters")[1]

     if hq and hq.descr.name == "barbarians_headquarters" then
       if hq:get_wares("water") < 100 then
         hq:set_wares("water", hq:get_wares("water") + 20)
       end
       if hq:get_wares("log") < 100 then
         hq:set_wares("log", hq:get_wares("log") + 20)
       end
       if hq:get_wares("granite") < 100 then
         hq:set_wares("granite", hq:get_wares("granite") +   10)
       end
       if hq:get_wares("coal") < 100 then
         hq:set_wares("coal", hq:get_wares("coal") + 5)
       end
       if hq:get_wares("iron_ore") < 100 then
         hq:set_wares("iron_ore", hq:get_wares("iron_ore") + 5)
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

return init

