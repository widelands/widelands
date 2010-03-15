-- =======================================================================
--                    Start conditions for Citadel Village
-- =======================================================================

use("aux", "infrastructure")

function initialize(p) 
   local sf = p.starting_field

   -- TODO: SirVer, Lua: allow worker types

   hq = prefilled_buildings(p, { "headquarters", sf.x, sf.y,
      wares = {
         axe = 6,
         bakingtray = 2,
         blackwood = 32,
         cloth = 5,
         coal = 12,
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
         trunk = 80,
      },
      workers = {
         blacksmith = 2,
         brewer = 1,
         builder = 10,
         burner = 1,
         carrier = 40,
         ferner = 1,
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


return initialize
