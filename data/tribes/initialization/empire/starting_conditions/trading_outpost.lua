-- =======================================================================
--           Trading Outpost starting conditions for Empire
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Trading Outpost"),
   -- TRANSLATORS: This is the tooltip for the "Trading Outpost" starting condition
   tooltip = _("If this player runs low on important wares, they will be replenished for free"),
   func = function(player, shared_in_start)

      local sf = wl.Game().map.player_slots[player.number].starting_field

      if shared_in_start then
         sf = shared_in_start
      else
         player:allow_workers("all")
      end

      prefilled_buildings(player, { "empire_headquarters", sf.x, sf.y,
         wares = {
            armor_helmet = 4,
            spear_wooden = 5,
            felling_ax = 6,
            bread_paddle = 2,
            basket = 2,
            empire_bread = 8,
            cloth = 5,
            coal = 12,
            fire_tongs = 2,
            fish = 6,
            fishing_rod = 2,
            flour = 4,
            gold = 14,
            grape = 4,
            hammer = 14,
            hunting_spear = 2,
            iron = 12,
            iron_ore = 5,
            kitchen_tools = 4,
            marble = 25,
            marble_column = 6,
            meal = 4,
            meat = 6,
            pick = 8,
            ration = 12,
            saw = 2,
            scythe = 5,
            shovel = 6,
            granite = 40,
            log = 80,
            water = 12,
            wheat = 4,
            wine = 8,
            planks = 45,
            wool = 2,
         },
         workers = {
            empire_armorsmith = 1,
            empire_brewer = 1,
            empire_builder = 10,
            empire_carrier = 40,
            empire_charcoal_burner = 1,
            empire_geologist = 4,
            empire_lumberjack = 3,
            empire_miner = 4,
            empire_stonemason = 2,
            empire_toolsmith = 2,
            empire_trainer = 4,
            empire_weaponsmith = 1,
            empire_donkey = 5,
         },
         -- Max health: 4, Max attack: 4, Max defense: 0, Max evade: 2
         soldiers = {
            [{0,0,0,0}] = 35,
            [{1,0,0,1}] = 5,
            [{1,1,0,0}] = 5,
         }
      })

      place_building_in_region(player, "empire_toolsmithy", sf:region(11), {
         inputs = {
            iron = 8,
            log = 8
         }
      })

      place_building_in_region(player, "empire_sawmill", sf:region(11), {
         inputs = {
            log = 8
         }
      })

      place_building_in_region(player, "empire_hunters_house", sf:region(11), {
         inputs = {}
      })

      place_building_in_region(player, "empire_tower", sf:region(13), {
         soldiers = {
            [{0,0,0,0}] = 1,
         },
      })

      -- Get all warehouse types
      local warehouse_types = {}
      for i, building in ipairs(wl.Game():get_tribe_description(player.tribe_name).buildings) do
         if (building.type_name == "warehouse") then
            table.insert(warehouse_types, building.name)
         end
      end

      -- index of a warehouse we will add to. Used to 'rotate' warehouses
      local idx = 1

      for i=1,100000 do
      sleep(300000)

      -- collect all ~warehouses and pick one to insert the wares
      local warehouses = {}
      for i, building_name in ipairs(warehouse_types) do
            warehouses = array_combine(warehouses, player:get_buildings(building_name))
      end

      if #warehouses > 0 then

         -- adding to a warehouse with index idx, if out of range, adding to wh 1
          if idx > #warehouses then
            idx = 1
         end

         local wh = warehouses[idx]
         local added = 0

         if wh:get_wares("water") < 100 then
            wh:set_wares("water", wh:get_wares("water") + 20)
         end
         if wh:get_wares("log") < 100 then
            wh:set_wares("log", wh:get_wares("log") + 10)
         end
         if wh:get_wares("granite") < 100 then
            wh:set_wares("granite", wh:get_wares("granite") + 5 + #warehouses)
         end
         if wh:get_wares("coal") < 100 then
            wh:set_wares("coal", wh:get_wares("coal") + 5)
         end
         if wh:get_wares("iron_ore") < 100 then
            wh:set_wares("iron_ore", wh:get_wares("iron_ore") + 5)
         end
         if wh:get_wares("marble") < 100 then
            wh:set_wares("marble", wh:get_wares("marble") + 5)
         end
         if wh:get_wares("fish") < 50 then
            wh:set_wares("fish", wh:get_wares("fish") + 1)
         end
         if wh:get_wares("gold") < 50 then
            wh:set_wares("gold", wh:get_wares("gold") + 1)
         end
         if player:get_wares("wheat") < 60 + #warehouses * 10 then
            wh:set_wares("wheat", wh:get_wares("wheat") + 10 + #warehouses * 2)
            added = added + 1
         end
         if player:get_wares("flour") < 30 + #warehouses * 10 then
            wh:set_wares("flour", wh:get_wares("flour") + #warehouses * 5)
            added = added + 1
         end

         if (added > 0) then
            print (player.number..": "..added.." types of ware added to warehouse "..idx.." of "..#warehouses.." (cheating mode)")
         end

         idx = idx + 1
      end
   end
end
}
pop_textdomain()
return init
