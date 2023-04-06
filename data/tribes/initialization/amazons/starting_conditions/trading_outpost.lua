-- =======================================================================
--           Trading Outpost starting conditions for Amazons
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

      prefilled_buildings(player, { "amazons_headquarters", sf.x, sf.y,
         wares = {
         log = 120,
         granite = 50,
         rope = 30,
         liana = 35,
         ironwood = 15,
         rubber = 15,
         balsa = 9,
         water = 20,
         fish = 10,
         meat = 10,
         cassavaroot = 15,
         ration = 5,
         coal = 5,
         gold = 14,
         amazons_bread = 5,
         chocolate = 6,
         hammer = 10,
         shovel = 5,
         cocoa_beans = 3,
         tunic = 5,
         chisel = 3,
         pick = 2,
         machete = 2,
         spear_wooden = 6,
         stone_bowl = 2,
         needles = 2,
         },
         workers = {
         amazons_cook = 3,
         amazons_builder = 10,
         amazons_gold_digger = 1,
         amazons_gold_smelter = 1,
         amazons_carrier = 40,
         amazons_cassava_farmer = 3,
         amazons_cocoa_farmer = 3,
         amazons_stone_carver = 3,
         amazons_geologist = 4,
         amazons_woodcutter = 10,
         amazons_liana_cutter = 4,
         amazons_jungle_preserver = 5,
         amazons_jungle_master = 3,
         amazons_shipwright = 1,
         amazons_hunter_gatherer = 3,
         amazons_stonecutter = 2,
         amazons_trainer = 4,
         amazons_dressmaker = 1,
         amazons_tapir = 5,
         amazons_woodcutter_master = 3,
         },
         -- Max health: 3, Max attack: 3, Max defense: 3, Max evade: 3
         soldiers = {
            [{0,0,0,0}] = 35,
            [{1,0,0,1}] = 5,
            [{1,1,0,0}] = 5,
         }
      })

      place_building_in_region(player, "amazons_stone_workshop", sf:region(11), {
         inputs = {
            granite = 7,
            log = 6,
            ironwood = 5,
         }
      })

      place_building_in_region(player, "amazons_rope_weaver_booth", sf:region(11), {
         inputs = {
            liana = 8,
         }
      })

      place_building_in_region(player, "amazons_hunter_gatherers_hut", sf:region(11), {
         inputs = {}
      })

      place_building_in_region(player, "amazons_observation_tower", sf:region(13), {
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
            wh:set_wares("log", wh:get_wares("log") + 10)
            added = added + 1
         end
         if wh:get_wares("granite") < 100 then
            wh:set_wares("granite", wh:get_wares("granite") + 5)
            added = added + 1
         end
         if wh:get_wares("coal") < 100 then
            wh:set_wares("coal", wh:get_wares("coal") + 10)
            added = added + 1
         end
         if wh:get_wares("ironwood") < 30 then
            wh:set_wares("ironwood", wh:get_wares("ironwood") + 5)
            added = added + 1
         end
         if wh:get_wares("rubber") < 30 then
            wh:set_wares("rubber", wh:get_wares("rubber") + 5)
            added = added + 1
         end
         if wh:get_wares("liana") < 100 then
            wh:set_wares("liana", wh:get_wares("liana") + 5)
            added = added + 1
         end
         if wh:get_wares("fish") < 50 then
            wh:set_wares("fish", wh:get_wares("fish") + 1)
            added = added + 1
         end
         if wh:get_wares("gold") < 50 then
            wh:set_wares("gold", wh:get_wares("gold") + 1)
            added = added + 1
         end
         if player:get_wares("cassavaroot") < 60 then
            wh:set_wares("cassavaroot", wh:get_wares("cassavaroot") + 10)
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
