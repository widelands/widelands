-- =======================================================================
--          Trading Outpost starting conditions for Atlanteans
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Trading Outpost"),
   -- TRANSLATORS: This is the tooltip for the "Trading Outpost" starting condition
   tooltip = _("If this player runs low on important wares, they will be replenished for free"),
   func =  function(player, shared_in_start)

      local sf = wl.Game().map.player_slots[player.number].starting_field
      if shared_in_start then
         sf = shared_in_start
      else
         player:allow_workers("all")
      end

      prefilled_buildings(player, { "atlanteans_headquarters", sf.x, sf.y,
         wares = {
            diamond = 7,
            iron_ore = 5,
            quartz = 9,
            granite = 50,
            spider_silk = 9,
            log = 80,
            coal = 12,
            gold = 14,
            gold_thread = 6,
            iron = 8,
            planks = 45,
            spidercloth = 5,
            blackroot = 5,
            blackroot_flour = 12,
            atlanteans_bread = 8,
            corn = 5,
            cornmeal = 12,
            fish = 3,
            meat = 3,
            smoked_fish = 6,
            smoked_meat = 6,
            water = 12,
            bread_paddle = 2,
            buckets = 2,
            fire_tongs = 2,
            fishing_net = 4,
            hammer = 11,
            hunting_bow = 1,
            milking_tongs = 2,
            hook_pole = 2,
            pick = 8,
            saw = 9,
            scythe = 4,
            shovel = 9,
            tabard = 5,
            trident_light = 5,
         },
         workers = {
            atlanteans_armorsmith = 1,
            atlanteans_blackroot_farmer = 1,
            atlanteans_builder = 10,
            atlanteans_charcoal_burner = 1,
            atlanteans_carrier = 40,
            atlanteans_fishbreeder = 1,
            atlanteans_geologist = 4,
            atlanteans_miner = 4,
            atlanteans_sawyer = 1,
            atlanteans_stonecutter = 2,
            atlanteans_toolsmith = 2,
            atlanteans_trainer = 4,
            atlanteans_weaponsmith = 1,
            atlanteans_woodcutter = 3,
            atlanteans_horse = 5,
         },
         -- Max health: 1, Max attack: 4, Max defense: 2, Max evade: 2
         soldiers = {
            [{0,0,0,0}] = 25,
            [{1,0,0,1}] = 5,
            [{1,1,0,0}] = 5,
         }
      })

      place_building_in_region(player, "atlanteans_toolsmithy", sf:region(11), {
         inputs = {
            iron = 6,
            log = 6,
            spidercloth = 4
         }
      })

      place_building_in_region(player, "atlanteans_sawmill", sf:region(11), {
         inputs = {
            log = 8
         }
      })

      place_building_in_region(player, "atlanteans_hunters_house", sf:region(11), {
         inputs = {}
      })

      place_building_in_region(player, "atlanteans_tower", sf:region(13), {
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
            added = added + 1
         end
         if wh:get_wares("log") < 100 then
            wh:set_wares("log", wh:get_wares("log") + 20)
            added = added + 1
         end
         if wh:get_wares("granite") < 100 then
            wh:set_wares("granite", wh:get_wares("granite") + 10)
            added = added + 1
         end
         if wh:get_wares("coal") < 100 then
            wh:set_wares("coal", wh:get_wares("coal") + 5)
            added = added + 1
         end
         if wh:get_wares("iron_ore") < 100 then
            wh:set_wares("iron_ore", wh:get_wares("iron_ore") + 5)
            added = added + 1
         end
         if wh:get_wares("quartz") < 10 then
            wh:set_wares("quartz", wh:get_wares("quartz") + 1)
            added = added + 1
         end
         if wh:get_wares("meat") < 50 then
            wh:set_wares("meat", wh:get_wares("meat") + 1)
            added = added + 1
         end
         if wh:get_wares("gold") < 10 then
            wh:set_wares("gold", wh:get_wares("gold") + 1)
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
