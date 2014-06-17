-- =======================================================================
--                    Start conditions for Citadel Village
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribe_barbarians")

return {
   name = _ "Citadel village",
   func =  function(plr, shared_in_start)

   local sf = wl.Game().map.player_slots[plr.number].starting_field

   if shared_in_start then
      sf = shared_in_start
   else
      plr:allow_workers("all")
   end

   local h = plr:place_building("citadel", sf, false, true)
   h:set_soldiers{[{0,0,0,0}] = 12}

   if not pcall(function()
       place_building_in_region(plr, "warehouse", sf:region(7), {
         wares = {
            ax = 5,
            bread_paddle = 2,
            blackwood = 32,
            cloth = 5,
            coal = 1,
            felling_ax = 4,
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
            log = 79,
         },
         workers = {
            brewer = 1,
            builder = 10,
            burner = 1,
            carrier = 38,
            gardener = 1,
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

      place_building_in_region(plr, "battlearena", sf:region(12), {
         wares = {
            pittabread = 8,
            fish = 6,
            meat = 6,
         }
      })

      place_building_in_region(plr, "trainingcamp", sf:region(12))

      place_building_in_region(plr, "helmsmithy", sf:region(12), {
         wares = { iron = 4, gold = 4 }
      })
      place_building_in_region(plr, "metalworks", sf:region(12), {
         wares = { iron = 8 },
      })
      place_building_in_region(plr, "axfactory", sf:region(12), {
         wares = { coal = 8 },
      })
      place_building_in_region(plr, "hardener", sf:region(12), {
         wares = { log = 1 },
      })
      place_building_in_region(plr, "lime_kiln", sf:region(12), {
         wares = { raw_stone = 6, coal = 3 },
      })
   end) then
      plr:send_message(_"Not enough space", _(
[[Some of your starting buildings didn’t have enough room and ]] ..
[[weren’t built. You are at a disadvantage with this; consider restarting ]] ..
[[this map with a fair starting condition.]]), {popup=true}
      )
   end
end,
}
