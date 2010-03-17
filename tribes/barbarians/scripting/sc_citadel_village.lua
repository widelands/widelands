-- =======================================================================
--                    Start conditions for Citadel Village
-- =======================================================================

use("aux", "infrastructure")

function initialize(p) 

   p:allow_workers("all")
   
   local sf = p.starting_field
   local h = p:place_building("citadel", sf)
   h:warp_soldiers{[{0,0,0,0}] = 12}

    place_building_in_region(p, "warehouse", sf:region(7), {
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
         [{0,0,0,0}] = 25, 
      }
   })

   place_building_in_region(p, "battlearena", sf:region(12), {
      wares = {
         pittabread = 8,
         fish = 6,
         meat = 6,
      }
   })
   
   place_building_in_region(p, "trainingscamp", sf:region(12))
  
   place_building_in_region(p, "helmsmithy", sf:region(12), {
      wares = { iron = 4, gold = 4 }
   })
   place_building_in_region(p, "metalworks", sf:region(12), {
      wares = { iron = 8 },
   })
   place_building_in_region(p, "axefactory", sf:region(12), {
      wares = { coal = 8 },
   })
   place_building_in_region(p, "hardener", sf:region(12), {
      wares = { trunk = 1 },
   })
   place_building_in_region(p, "lime_kiln", sf:region(12), {
      wares = { raw_stone = 6, coal = 3 },
   })

end


return initialize
