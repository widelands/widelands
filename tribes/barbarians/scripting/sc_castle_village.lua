-- =======================================================================
--                    Start conditions for Citadel Village
-- =======================================================================

use("aux", "coroutine")
use("aux", "infrastructure")

function _place_nearby(plr, building, fields, required_suitability, args)
   local idx
   local f

   while #fields > 0 do
      local idx = math.random(#fields)
      local f = fields[idx]

      if plr:get_suitability(building, f) >= required_suitability then
         args[1] = building
         args[2] = f.x
         args[3] = f.y
         return prefilled_buildings(plr, args)
      end
      table.remove(fields, idx)
   end
   error(string.format(
      "Could not find a suitable position for '%s'", building)
   )
end

function initialize(p) 

   p:allow_all_workers();
   
   local sf = p.starting_field
   local h = p:place_building("citadel", sf)
   h:warp_soldiers{[{0,0,0,0}] = 12}

    _place_nearby(p, "warehouse", sf:region(11), 1, {
      wares = {
         axe = 6,
         bakingtray = 2,
         blackwood = 32,
         cloth = 5,
         coal = 1,
         fire_tongs = 2,
         fishing_rod = 2,
         grout = 12,
         hammer = 11,
         hunting_spear = 2,
         ironore = 5,
         kitchen_tools = 4,
         meal = 4,
         pick = 14,
         ration = 12,
         raw_stone = 34,
         scythe = 6,
         shovel = 4,
         snack = 3,
         thatchreed = 24,
         trunk = 79,
      }, 
      workers = {
         brewer = 1,
         builder = 10,
         burner = 1,
         carrier = 38,
         ferner = 1,
         geologist = 4,
         lumberjack = 2,
         miner = 4,
         ranger = 1,
         stonemason = 2,
         ox = 5,
      },
      soldiers = {
         [{0,0,0,0}] = 13, 
      }
   })

   -- _place_nearby(p, "battlearena", sf:region(20), 1, {
   --    wares = {
   --       pittabread = 8,
   --       fish = 6,
   --       meat = 6,
   --    }
   -- })
   


end


return initialize
