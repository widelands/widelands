-- =======================================================================
--                 Empire Castle Village Starting Conditions
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribe_empire")

return {
   name = _ "Castle village",
   func =  function(plr, shared_in_start)

   local sf = wl.Game().map.player_slots[plr.number].starting_field

   if shared_in_start then
      sf = shared_in_start
   else
      plr:allow_workers("all")
   end

   local h = plr:place_building("castle", sf, false, true)
   h:set_soldiers{[{0,0,0,0}] = 12}

   if not pcall(function()
      place_building_in_region(plr, "warehouse", sf:region(7), {
         wares = {
            helm = 2,
            wood_lance = 5,
            ax = 6,
            bread_paddle = 2,
            basket = 2,
            fire_tongs = 2,
            fishing_rod = 2,
            flour = 4,
            grape = 4,
            hammer = 14,
            hunting_spear = 2,
            iron = 4,
            ironore = 5,
            kitchen_tools = 4,
            marble = 25,
            marblecolumn = 6,
            meal = 4,
            pick = 14,
            ration = 12,
            saw = 2,
            scythe = 5,
            shovel = 6,
            stone = 40,
            log = 29,
            water = 12,
            wheat = 4,
            wine = 8,
            wood = 37,
            wool = 2,
         },
         workers = {
            brewer = 1,
            builder = 10,
            burner = 1,
            carrier = 39,
            geologist = 4,
            lumberjack = 3,
            miner = 4,
            stonemason = 1,
            toolsmith = 1,
            donkey = 5,
         },
         soldiers = {
            [{0,0,0,0}] = 33,
         }
      })

      place_building_in_region(plr, "colosseum", sf:region(11), {
         wares = {
            bread = 8,
            fish = 4,
            meat = 4,
         },
      })

      place_building_in_region(plr, "trainingcamp", sf:region(11), {
         wares = {
            fish = 2,
            meat = 2,
            helm = 2,
         },
      })

      place_building_in_region(plr, "armorsmithy", sf:region(11), {
         wares = {
               gold = 4,
               coal = 8,
               cloth = 5,
         }
      })

      place_building_in_region(plr, "toolsmithy", sf:region(11), {
         wares = {
            iron = 8,
         }
      })

      place_building_in_region(plr, "weaponsmithy", sf:region(11), {
         wares = {
            coal = 4,
            wood = 8,
         }
      })

      place_building_in_region(plr, "sawmill", sf:region(11), {
         wares = {
            log = 1,
         }
      })

      place_building_in_region(plr, "stonemasons_house", sf:region(11))
   end) then
      plr:send_message(_"Not enough space", _(
[[Some of your starting buildings didn’t have enough room and ]] ..
[[weren’t built. You are at a disadvantage with this; consider restarting ]] ..
[[this map with a fair starting condition.]]), {popup=true}
      )
   end
end
}
