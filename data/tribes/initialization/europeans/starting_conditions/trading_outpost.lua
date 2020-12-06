-- =======================================================================
--          Trading Outpost Starting Conditions for the Europeans
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

return {
    -- TRANSLATORS: This is the name of a starting condition
    descname = _"Trading Outpost",
    -- TRANSLATORS: This is the tooltip for the "Trading Outpost" starting condition
    tooltip = _"If this player runs low on important wares, they will be replenished for free",
    func =  function(plr, shared_in_start)

    local sf = wl.Game().map.player_slots[plr.number].starting_field
    if shared_in_start then
     sf = shared_in_start
    else
     plr:allow_workers("all")
    end

    prefilled_buildings(plr, { "europeans_headquarters", sf.x, sf.y,
        wares = {
            water = 128,
            log = 64,
            planks = 32,
            spidercloth = 32,
            spider_silk = 32,
            reed = 32,
            coal = 64,
            granite = 64,
            grout = 32,
            brick = 32,
            marble = 16,
            diamond = 16,
            quartz = 16,
            ore = 16,
        },
        workers = {
            europeans_carrier = 32,
            europeans_builder = 10,
            europeans_geologist = 2,
            europeans_trainer = 4,
            europeans_lumberjack_basic = 4,
            europeans_forester_basic = 2,
            europeans_carpenter_basic = 2,
            europeans_stonecutter_basic = 2,
            europeans_stonemason_basic = 2,
            europeans_hunter_basic = 2,
            europeans_fisher_basic = 2,
            europeans_fishbreeder = 2,
            europeans_farmer_basic = 4,
            europeans_miller_basic = 1,
            europeans_baker_basic = 1,
            europeans_smoker_basic = 1,
            europeans_brewer_basic = 1,
            europeans_breeder_normal = 1,
            europeans_weaver_basic = 1,
            europeans_charcoal_burner_basic = 1,
            europeans_miner_basic = 6,
            europeans_smelter_basic = 1,
            europeans_smith_basic = 2,
            europeans_shipwright = 2,
        },
        soldiers = {
            [{0,0,0,0}] = 25,
            [{1,0,0,1}] = 5,
            [{1,1,0,0}] = 5,
        }
      })

    place_building_in_region(plr, "europeans_well_advanced", sf:region(6), {
    })

      -- Get all warehouse types
      local warehouse_types = {}
      for i, building in ipairs(wl.Game():get_tribe_description(plr.tribe_name).buildings) do
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
            warehouses = array_combine(warehouses, plr:get_buildings(building_name))
      end

      if #warehouses > 0 then

         -- adding to a warehouse with index idx, if out of range, adding to wh 1
         if idx > #warehouses then
         idx = 1
         end

         local wh = warehouses[idx]
         local added = 0

         if wh:get_wares("coal") < 24 then
            wh:set_wares("coal", wh:get_wares("coal") + 4)
            added = added + 1
         end
         if wh:get_wares("ore") < 24 then
            wh:set_wares("ore", wh:get_wares("ore") + 4)
            added = added + 1
         end
         if wh:get_wares("granite") < 12 then
            wh:set_wares("granite", wh:get_wares("granite") + 2)
            added = added + 1
         end
         if wh:get_wares("clay") < 12 then
            wh:set_wares("clay", wh:get_wares("clay") + 2)
            added = added + 1
         end
         if wh:get_wares("marble") < 12 then
            wh:set_wares("marble", wh:get_wares("marble") + 2)
            added = added + 1
         end
         if wh:get_wares("quartz") < 12 then
            wh:set_wares("quartz", wh:get_wares("quartz") + 2)
            added = added + 1
         end
         if wh:get_wares("diamond") < 12 then
            wh:set_wares("diamond", wh:get_wares("diamond") + 2)
            added = added + 1
         end
         if wh:get_wares("log") < 12 then
            wh:set_wares("log", wh:get_wares("log") + 2)
            added = added + 1
         end
         if wh:get_wares("reed") < 12 then
            wh:set_wares("reed", wh:get_wares("reed") + 2)
            added = added + 1
         end
         if wh:get_wares("water") < 12 then
            wh:set_wares("water", wh:get_wares("water") + 2)
            added = added + 1
         end
         if wh:get_wares("hammer") < 1 then
            wh:set_wares("hammer", wh:get_wares("hammer") + 1)
            added = added + 1
         end
         if wh:get_wares("felling_ax") < 1 then
            wh:set_wares("felling_ax", wh:get_wares("felling_ax") + 1)
            added = added + 1
         end
         if wh:get_wares("saw") < 1 then
            wh:set_wares("saw", wh:get_wares("saw") + 1)
            added = added + 1
         end
         if wh:get_wares("shovel") < 1 then
            wh:set_wares("shovel", wh:get_wares("shovel") + 1)
            added = added + 1
         end
         if wh:get_wares("pick") < 1 then
            wh:set_wares("pick", wh:get_wares("pick") + 1)
            added = added + 1
         end
         if wh:get_wares("fire_tongs") < 1 then
            wh:set_wares("fire_tongs", wh:get_wares("fire_tongs") + 1)
            added = added + 1
         end
         if (added > 0) then
            print (plr.number..": "..added.." types of ware added to warehouse "..idx.." of "..#warehouses.." (cheating mode)")
         end

         idx = idx + 1
      end
   end
end
}
