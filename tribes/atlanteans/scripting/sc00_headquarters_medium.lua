-- =======================================================================
--              Headquarters Starting Conditions for Atlanteans             
-- =======================================================================

use("aux", "infrastructure")
use("aux", "shared_kingdom_functions")

set_textdomain("tribe_atlanteans")

init = {
   name = _ "Headquarters medium", 
   func =  function(plr) 
   plr:allow_workers("all")

   local sf = wl.Game().map.player_slots[plr.number].starting_field

   plr = actual_player(plr)

   prefilled_buildings(plr, { "headquarters", sf.x, sf.y, 
      wares = {
         diamond = 7,
         ironore = 5,
         quartz = 9,
         stone = 50,
         spideryarn = 9,
         trunk = 20,
         coal = 12,
         gold = 4,
         goldyarn = 6,
         iron = 8,
         planks = 45,
         spidercloth = 5,
         blackroot = 5,
         blackrootflour = 12,
         bread = 8,
         corn = 5,
         cornflour = 12,
         fish = 3,
         meat = 3,
         smoked_fish = 6,
         smoked_meat = 6,
         water = 12,
         bakingtray = 2,
         bucket = 2,
         fire_tongs = 2,
         fishing_net = 4,
         hammer = 11,
         hunting_bow = 1,
         milking_tongs = 2,
         hook_pole = 2,
         pick = 12,
         saw = 9,
         scythe = 4,
         shovel = 9,
         tabard = 5,
         light_trident = 5,
      },
      workers = {
         armoursmith = 1,
         blackroot_farmer = 1,
         builder = 10,
         burner = 1,
         carrier = 40,
         fish_breeder = 1,
         geologist = 4,
         miner = 4,
         sawyer = 1,
         stonecutter = 2,
         toolsmith = 2,
         weaponsmith = 1,
         woodcutter = 3,
         horse = 5,
      },
      soldiers = {
         [{0,0,0,0}] = 35,
      }
   })
end
}

return init

