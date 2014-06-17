-- =======================================================================
--                    Castle Village Starting Conditions
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribe_atlanteans")

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
            diamond = 7,
            ironore = 5,
            quartz = 9,
            stone = 50,
            spideryarn = 9,
            log = 13,
            goldyarn = 6,
            planks = 45,
            spidercloth = 5,
            blackroot = 5,
            blackrootflour = 12,
            corn = 5,
            cornflour = 12,
            fish = 3,
            meat = 3,
            water = 12,
            bread_paddle = 2,
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
            blackroot_farmer = 1,
            builder = 10,
            burner = 1,
            carrier = 38,
            fish_breeder = 1,
            geologist = 4,
            miner = 4,
            stonecutter = 2,
            toolsmith = 1,
            woodcutter = 3,
            horse = 5,
         },
         soldiers = {
            [{0,0,0,0}] = 23,
         },
      })

      place_building_in_region(plr, "labyrinth", sf:region(11), {
         wares = {
            bread = 4,
            smoked_fish = 3,
            smoked_meat = 3,
         }
      })

      place_building_in_region(plr, "dungeon", sf:region(11), {
         wares = {bread = 4, smoked_fish = 3, smoked_meat = 3}
      })

      place_building_in_region(plr, "armorsmithy", sf:region(11), {
         wares = { coal=4, gold =4 }
      })
      place_building_in_region(plr, "toolsmithy", sf:region(11), {
         wares = { log = 6 }
      })
      place_building_in_region(plr, "weaponsmithy", sf:region(11), {
         wares = { coal = 8, iron = 8 }
      })

      place_building_in_region(plr, "sawmill", sf:region(11), {
         wares = { log = 1 }
      })
   end) then
      plr:send_message(_"Not enough space", _(
[[Some of your starting buildings didn’t have enough room and ]] ..
[[weren’t built. You are at a disadvantage with this; consider restarting ]] ..
[[this map with a fair starting condition.]]), {popup=true}
      )
   end

end
}
