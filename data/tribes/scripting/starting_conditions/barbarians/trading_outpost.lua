-- =======================================================================
--                 Start conditions for Headquarters Medium
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

return {
   descname = _"Trading Outpost",
   tooltip = _"Never run out of important wares for long",
   func = function(player, shared_in_start)
   print (string.format(" %i: Initiating 'Headquarters cheat' mode",
   player.number))

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
         barbarians_blacksmith_master = 1,
         barbarians_brewer = 1,
         barbarians_brewer_master = 1,
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
   place_building_in_region(player, "barbarians_tower", sf:region(13), {
      soldiers = {
         [{0,0,0,0}] = 1,
      },
      })
   place_building_in_region(player, "barbarians_battlearena", sf:region(11), {
        wares = {
            barbarians_bread = 8,
            fish = 6,
            meat = 6,
         }
      })
   place_building_in_region(player, "barbarians_brewery", sf:region(11), {
      wares = {},
      })

    local plr = wl.Game().players[player.number]
    -- index of a warehouse we will add to. Used to 'rotate' warehouses
    local idx = 1

    for i=1,100000 do
      sleep(300000)

      -- collect all ~warehouses and pick one to insert the wares
      local warehouses = array_combine(plr:get_buildings(plr.tribe_name .. "_headquarters"),
         plr:get_buildings(plr.tribe_name .. "_warehouse"),
         plr:get_buildings(plr.tribe_name .. "_port"))

      if #warehouses > 0 then

         -- adding to a warehouse with index idx, if out of range, adding to wh 1
          if idx > #warehouses then
            idx = 1
         end

          local wh = warehouses[idx]
          local added = 0

          if plr:get_wares("water") < 40 + #warehouses * 10 then
              wh:set_wares("water", wh:get_wares("water") + 20 + #warehouses * 2)
              added = added + 1
          end
          if plr:get_wares("log") < 40 + #warehouses * 10 then
              wh:set_wares("log", wh:get_wares("log") + 20)
              added = added + 1
          end
          if plr:get_wares("granite") < 30 + #warehouses * 10 then
              wh:set_wares("granite", wh:get_wares("granite") +  10 + #warehouses * 2)
              added = added + 1
          end
          if plr:get_wares("coal") < 70 + #warehouses * 10 then
              wh:set_wares("coal", wh:get_wares("coal") + 25 + #warehouses * 5)
              added = added + 1
          end
          if plr:get_wares("iron_ore") < 30 + #warehouses * 10 then
              wh:set_wares("iron_ore", wh:get_wares("iron_ore") + 10 + #warehouses * 2)
              added = added + 1
          end
          if wh:get_wares("fish") < 40 + #warehouses * 5 then
              wh:set_wares("fish", wh:get_wares("fish") + 10)
            added = added + 1
          end
          if plr:get_wares("gold") < 20 + #warehouses * 5 then
              wh:set_wares("gold", wh:get_wares("gold") + 3)
              added = added + 1
          end
          if plr:get_wares("wheat") < 60 + #warehouses * 10 then
              wh:set_wares("wheat", wh:get_wares("wheat") + 15 + #warehouses * 2)
              added = added + 1
          end
          if plr:get_wares("barbarians_bread") < 40 + #warehouses * 5 then
              wh:set_wares("barbarians_bread", wh:get_wares("barbarians_bread") + 8 + #warehouses * 2)
              added = added + 1
          end

          print (player.number..": "..added.." types of ware added to warehouse "..idx.." of "..#warehouses.." (cheating mode)")

          idx = idx + 1
      end
    end
end
}
