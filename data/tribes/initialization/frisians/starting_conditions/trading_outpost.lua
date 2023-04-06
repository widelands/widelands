-- =======================================================================
--           Trading Outpost starting conditions for Frisians
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

      prefilled_buildings(player, { "frisians_headquarters", sf.x, sf.y,
         wares = {
             log = 30,
             granite = 45,
             reed = 35,
             brick = 70,
             clay = 20,
             water = 5,
             fish = 10,
             meat = 10,
             fruit = 10,
             barley = 5,
             ration = 20,
             honey = 10,
             smoked_meat = 5,
             smoked_fish = 5,
             mead = 5,
             meal = 2,
             coal = 15,
             gold = 4,
             iron_ore = 10,
             bread_frisians = 15,
             honey_bread = 5,
             beer = 5,
             cloth = 5,
             fur = 10,
             fur_garment = 5,
             sword_short = 5,
             hammer = 10,
             fire_tongs = 2,
             bread_paddle = 2,
             kitchen_tools = 2,
             felling_ax = 3,
             needles = 1,
             basket = 2,
             pick = 5,
             shovel = 5,
             scythe = 3,
             hunting_spear = 2,
             fishing_net = 3,
         },
         workers = {
             frisians_blacksmith = 3,
             frisians_baker = 1,
             frisians_brewer = 1,
             frisians_builder = 10,
             frisians_charcoal_burner = 1,
             frisians_claydigger = 2,
             frisians_brickmaker,
             frisians_carrier = 40,
             frisians_reed_farmer = 2,
             frisians_berry_farmer = 1,
             frisians_farmer = 1,
             frisians_landlady = 1,
             frisians_smoker = 1,
             frisians_geologist = 4,
             frisians_woodcutter = 3,
             frisians_beekeeper = 1,
             frisians_miner = 4,
             frisians_miner_master = 2,
             frisians_forester = 2,
             frisians_stonemason = 2,
             frisians_reindeer = 5,
             frisians_trainer = 4,
             frisians_seamstress_master = 1,
         },
         -- Max health: 2, Max attack: 6, Max defense: 2, Max evade: 0
         soldiers = {
            [{0,0,0,0}] = 34,
            [{1,0,1,0}] = 5,
            [{1,1,0,0}] = 5,
         }
      })

      place_building_in_region(player, "frisians_blacksmithy", sf:region(11), {
         inputs = {
            iron = 7,
            log = 7,
            reed = 4,
         }
      })

      place_building_in_region(player, "frisians_clay_pit", sf:region(11), {
         inputs = {
            water = 4
         }
      })

      place_building_in_region(player, "frisians_brick_kiln", sf:region(11), {
         inputs = {
            clay = 6,
            granite = 3,
            coal = 3
         }
      })

      place_building_in_region(player, "frisians_hunters_house", sf:region(11), {
         inputs = {}
      })

      place_building_in_region(player, "frisians_outpost", sf:region(13), {
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
         if wh:get_wares("reed") < 100 then
            wh:set_wares("reed", wh:get_wares("reed") + 5)
            added = added + 1
         end
         if wh:get_wares("coal") < 100 then
            wh:set_wares("coal", wh:get_wares("coal") + 10)
            added = added + 1
         end
         if wh:get_wares("iron_ore") < 100 then
            wh:set_wares("iron_ore", wh:get_wares("iron_ore") + 5)
            added = added + 1
         end
         if wh:get_wares("brick") < 100 then
            wh:set_wares("brick", wh:get_wares("brick") + 5)
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
         if player:get_wares("barley") < 60 then
            wh:set_wares("barley", wh:get_wares("barley") + 10)
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
