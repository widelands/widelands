-- =======================================================================
--                 Start conditions for Headquarters Medium
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribe_barbarians")

init = {
   name = _ "Headquarters medium",
   func = function(player, shared_in_start)

   local sf = wl.Game().map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   hq = prefilled_buildings(player, { "headquarters", sf.x, sf.y,
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
         ironore = 5,
         kitchen_tools = 4,
         meal = 4,
         meat = 6,
         pick = 14,
         pittabread = 8,
         ration = 12,
         raw_stone = 40,
         scythe = 6,
         shovel = 4,
         snack = 3,
         thatchreed = 24,
         log = 80,
      },
      workers = {
         blacksmith = 2,
         brewer = 1,
         builder = 10,
         burner = 1,
         carrier = 40,
         gardener = 1,
         geologist = 4,
         ["lime-burner"] = 1,
         lumberjack = 3,
         miner = 4,
         ranger = 1,
         stonemason = 2,
         ox = 5,
      },
      soldiers = {
         [{0,0,0,0}] = 45,
      }
   })
end
}

return init
