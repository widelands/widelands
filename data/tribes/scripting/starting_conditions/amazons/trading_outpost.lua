-- =======================================================================
--           Trading Outpost Starting Conditions for the amazons
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

return {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _"Trading Outpost",
   -- TRANSLATORS: This is the tooltip for the "Trading Outpost" starting condition
   tooltip = _"If this player runs low on important wares, they will be replenished for free",
   func = function(player, shared_in_start)

      local sf = wl.Game().map.player_slots[player.number].starting_field

      if shared_in_start then
         sf = shared_in_start
      else
         player:allow_workers("all")
      end

      prefilled_buildings(player, { "amazons_headquarters", sf.x, sf.y,
         wares = {
            log = 40,
            granite = 50,
            water = 10,
            fish = 10,
            meat = 10,
            cassava_root = 5,
            ration = 20,
            coal = 20,
            gold = 4,
            gold_dust = 5,
            bread_amazons = 15,
            hammer = 5,
            kitchen_tools = 2,
            felling_ax = 3,
            needles = 1,
            pick = 5,
            shovel = 5,
            hunting_spear = 2,
            fishing_net = 3,
            bread_amazons= 20,
            cocoa_beans = 5,
            chocolate= 15,
            tunic= 15,
            vest_padded= 2,
            protector_padded= 1,
            helmet_wooden= 1,
            boots_sturdy= 2,
            boots_swift= 1,
            boots_hero= 1,
            spear_wooden= 15,
            spear_stone_tipped= 1,
            spear_hardened= 1,
            armor_wooden= 2,
            warriors_coat= 1,
            stonebowl= 1,
            chisel= 2,
            rope= 20,
         },
         workers = {
            amazons_cook = 3,
            amazons_builder = 10,
            amazons_charcoal_burner = 1,
            amazons_gold_digger = 1,
            amazons_gold_smelter = 1,
            amazons_carrier = 40,
            amazons_cassava_farmer = 1,
            amazons_cocoa_farmer = 1,
            amazons_wilderness_keeper = 1,
            amazons_stonecarver = 2,
            amazons_geologist = 4,
            amazons_woodcutter = 3,
            amazons_liana_cutter = 1,
            amazons_jungle_preserver = 2,
            amazons_stonecutter = 2,
            amazons_trainer = 3,
            amazons_dressmaker = 1,
            amazons_tapir= 5,
         },
         -- Max health: 2, Max attack: 6, Max defense: 2, Max evade: 0
         soldiers = {
            [{0,0,0,0}] = 34,
            [{1,0,1,0}] = 5,
            [{1,1,0,0}] = 5,
         }
      })

      place_building_in_region(player, "amazons_stonecarvery", sf:region(11), {
         inputs = {
            ironwood = 4,
            log = 4,
            granite = 4
            rope = 4,
         }
      })

      place_building_in_region(player, "amazons_liana_cutters_hut", sf:region(11), {
         inputs = {
         }
      })

      place_building_in_region(player, "amazons_rope_weaver_booth", sf:region(11), {
         inputs = {
            liana = 7,
         }
      })

      place_building_in_region(player, "amazons_hunter_gatherer_hut", sf:region(11), {
         inputs = {}
      })

      place_building_in_region(player, "amazons_warriors_dwelling", sf:region(13), {
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
         if wh:get_wares("gold_dust") < 100 then
            wh:set_wares("gold_dust", wh:get_wares("gold_dust") + 5)
            added = added + 1
         end
         if wh:get_wares("rope") < 100 then
            wh:set_wares("rope", wh:get_wares("rope") + 5)
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
         if player:get_wares("cassava_root") < 60 then
            wh:set_wares("cassava_root", wh:get_wares("cassava_root") + 10)
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
