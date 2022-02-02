-- =======================================================================
--                             Along The River
-- =======================================================================

push_textdomain("map_along_the_river.wmf")

include "scripting/coroutine.lua"
include "scripting/field_animations.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/objective_utils.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

p1 = wl.Game().players[1]
p2 = wl.Game().players[2]
p3 = wl.Game().players[3]
p4 = wl.Game().players[4]
p5 = wl.Game().players[5]
--p1.see_all = true

-- Order is critical.
include "map:scripting/functions.lua"
include "map:scripting/objectives.lua"
include "map:scripting/texts.lua"
include "map:scripting/initial_conditions.lua"
include "map:scripting/swamps.lua"

-- =======================================================================
-- Building      Conquer range      Vision range
-- Sentry           6                     10
-- Barrier          9                     13
-- Tower            9                     17
-- Fortress        11                     15
-- Citadel         12                     16
-- =======================================================================

-- =======================================================================
-- Main mission thread
-- =======================================================================

function mission_thread()
   local map = wl.Game().map

   local hq = map:get_field(15, 119)
   local port = map:get_field(55, 38)
   local warehouse = map:get_field(58, 92)
   local port_island = map:get_field(76, 40)
   local stronghold = map:get_field(82, 29)

   p2:set_attack_forbidden(5, true)
   p5:set_attack_forbidden(2, true)
   p5:set_attack_forbidden(3, true)
   p5:set_attack_forbidden(4, true)
   p3:set_attack_forbidden(5, true)
   p3:set_attack_forbidden(4, true)
   p4:set_attack_forbidden(5, true)
   p4:set_attack_forbidden(3, true)

   connected_road("normal",p1, map:get_field(16, 120).immovable, "l,l,l")
   connected_road("normal",p1, map:get_field(13, 120).immovable, "tr,tr|tr,tr|tr,tr|tr,tr")
   connected_road("normal",p1, map:get_field(16, 120).immovable, "tr,tr|tr,tr|tr,tr|tr,tr")
   connected_road("normal",p1, map:get_field(20, 112).immovable, "tr,tr|tr,tr|tr,tr|tr,tr")
   connected_road("normal",p1, map:get_field(20, 112).immovable, "l,l,l")
   connected_road("normal",p1, map:get_field(24, 104).immovable, "l,tl,tr")
   connected_road("normal",p1, map:get_field(16, 120).immovable, "r,r")
   connected_road("normal",p1, map:get_field(18, 120).immovable, "tr,tr|tr,tr")
   connected_road("normal",p1, map:get_field(18, 120).immovable, "r,r|br,br|br,br|br,br|br,br|br,r|r,r")
   connected_road("normal",p1, map:get_field(27, 129).immovable, "br,r")

   sleep(500)
   campaign_message_box(briefing_intro)

   -- Prevent delays caused by soldier exchanges at the beginning of the game.
   run(function() spawn_soldiers(map:get_field( 26,  59), {1,1,0,0}, 20, 4999) end)   -- Warehouse p2
   run(function() spawn_soldiers(map:get_field(123, 119), {2,3,0,2}, 25, 5003) end)   -- HQ p3
   run(function() spawn_soldiers(map:get_field(109, 136), {2,3,0,2}, 25, 5009) end)   -- Warehouse p3
   run(function() spawn_soldiers(map:get_field(102,  36), {2,3,0,0}, 45, 5011) end)   -- HQ p4
   run(function() spawn_soldiers(map:get_field( 87,  65), {0,1,0,2}, 20, 5021) end)   -- HQ p5 I
   run(function() spawn_soldiers(map:get_field(134,  65), {0,2,0,2}, 20, 5023) end)   -- HQ p5 II

   -- =======================================================================
   -- Defeat p2 and build port
   -- Conquer pirate barbarians_sentry and build warehouse.
   -- =======================================================================

   run(function()
      sleep(600000)
      campaign_message_box(briefing_warn_for_red)

      -- Place fortress CS and sentries for p2.
      sleep(480000)
      local fortress = map:get_field(19, 70)
      p2:place_building("barbarians_fortress", fortress, true, true)
      -- Give time to start building.
      sleep(120000)
      place_building_if_owner(p2, "barbarians_sentry", map:get_field(19, 83), true, true)
      place_building_if_owner(p2, "barbarians_sentry", map:get_field(22, 85), true, true)
      place_building_if_owner(p2, "barbarians_sentry", map:get_field(25, 87), true, true)

      -- Inform p1 about new fortress CS and advancing border.
      -- Order to defeat p2.
      p1:reveal_fields(fortress:region(6))
      scroll_to_field(fortress)
      campaign_message_box(briefing_defeat_red)
      local o_defeat_red = add_campaign_objective(obj_defeat_red)
      run(function()
         sleep(30000)
         p1:hide_fields(fortress:region(6))
      end)

      sleep(600000)
      place_building_if_owner(p2, "barbarians_barrier", map:get_field(16, 86), true, true)

      run(function()
         sleep(1800000)
         p2:allow_buildings{"barbarians_sentry", "barbarians_barrier"}
         sleep(1800000)
         p2:allow_buildings{"barbarians_tower", "barbarians_fortress"}
      end)

      run(function()
         -- If citadel captured, fill HQ with elite soldiers.
         while map:get_field(33,50).owner == p2 do sleep(991) end
         campaign_message_box(briefing_red_mercenaries)
         local hq2 = map:get_field(33, 17).immovable
         if hq2 then hq2:set_soldiers({1,4,0,0}, hq2:get_soldiers({1,4,0,0}) + 16) end
      end)

      -- When p2 is defeated, order to build port.
      while not p2.defeated do sleep(2003) end
      map:get_field(32,1).terr = "summer_mountain1"
      map:get_field(32,0).terd = "summer_mountain3"
      map:get_field(30,143).terr = "summer_mountain1"
      p1:reveal_fields(port:region(6))
      scroll_to_field(port)
      campaign_message_box(briefing_build_port)
      o_build_port = add_campaign_objective(obj_build_port)
      o_defeat_red.done = true
      sleep(30000)
      map:get_field(31,1).terr = "summer_mountain2"
      map:get_field(30,142).terr = "summer_mountain4"
      map:get_field(30,142).terd = "summer_mountain1"
      p1:hide_fields(port:region(6))
      local pirate_fortress = map:get_field(55, 60)
      if pirate_fortress.immovable and pirate_fortress.owner == p5 then
         -- Soldier level must match with level in initial_conditions.
         pirate_fortress.immovable:set_soldiers({0,2,0,2}, 8)
      end
      while not immovable_has_name(port.immovable, "barbarians_port") do sleep(2039) end
      o_build_port.done = true
      map:get_field(30,143).terd = "summer_mountain2"
      map:get_field(29,143).terr = "summer_mountain3"
   end)

   -- Conquer pirate barbarians_sentry and build warehouse.
   run(function()
      local pirate_sentry = map:get_field(59, 89)
      while not p1:seen_field(pirate_sentry) do
         sleep(2011)
         -- Ai will dismantle sentry cause it can’t be connected to a WH so check and rebuild
         local sentry_immovable = pirate_sentry.immovable
         if not sentry_immovable or sentry_immovable.descr.name ~= "barbarians_sentry" then
            prefilled_buildings(p5, {"barbarians_sentry", 59, 89, soldiers = {[{0,2,0,2}] = 2 }})
         end
      end
      scroll_to_field(pirate_sentry)
      campaign_message_box(briefing_conquer_pirate_sentry)
      local o_conquer_pirate_sentry = add_campaign_objective(obj_conquer_pirate_sentry)

      while pirate_sentry.owner == p5 do sleep(997) end
      p1:place_flag(map:get_field(53, 87), true)
      map:get_field(53, 87).immovable:remove()
      p1:reveal_fields(warehouse:region(6))
      scroll_to_field(warehouse)
      campaign_message_box(briefing_found_pirate_treasure)
      o_build_warehouse = add_campaign_objective(obj_build_warehouse)
      o_conquer_pirate_sentry.done = true
      run(function()
         sleep(30000)
         p1:hide_fields(warehouse:region(6))
      end)
      transport_wares(p1, hq, "gold_ore", 25, true, "tribes/buildings/warehouses/barbarians/headquarters_interim/idle_00.png")
      while not o_build_warehouse.done do
         sleep(2053)
         for i, field in pairs(map:get_field(53,92):region(8)) do
            if immovable_has_name(field.immovable, "barbarians_warehouse") then
               warehouse = field
               o_build_warehouse.done = true
               break
            end
         end
      end
   end)

   while not (obj_done(o_build_warehouse) and obj_done(o_build_port)) do sleep(2063) end

   -- =======================================================================
   -- Bring shovels and remove the swamps.
   -- =======================================================================

   -- Wait for shovels in warehouse and hand them (done in "wares_collected_at_field" function)out.
   run(function()
      scroll_to_field(warehouse)
      campaign_message_box(briefing_bring_shovels_1)
      o_bring_shovels_1 = add_campaign_objective(obj_bring_shovels_1)
      while not wares_collected_at_field(warehouse, "shovel", 4) do sleep(2027) end
      push_textdomain("map_along_the_river.wmf")
      send_to_inbox(p1, _("Shovels"), li_image("tribes/buildings/warehouses/barbarians/warehouse/idle_1.png", _("Four shovels for removing the swamps have been collected at the warehouse.")), { field = warehouse, popup = true, })
      pop_textdomain()
      o_bring_shovels_1.done = true
      sleep(180000)
      scroll_to_field(warehouse)
      campaign_message_box(briefing_bring_shovels_2)
      o_bring_shovels_2 = add_campaign_objective(obj_bring_shovels_2)
      while not wares_collected_at_field(warehouse, "shovel", 4) do sleep(2027) end
      push_textdomain("map_along_the_river.wmf")
      send_to_inbox(p1, _("Shovels"), li_image("tribes/buildings/warehouses/barbarians/warehouse/idle_1.png", _("Four more shovels for removing the swamps have been collected at the warehouse.")), { field = warehouse, popup = true, })
      pop_textdomain()
      remove_swamps(p1)
      sleep(198000)
      -- After the work is done the shovels are returned to the Wh.
      transport_wares(p1, warehouse, "shovel", 8, true, "tribes/buildings/warehouses/barbarians/warehouse/menu.png")
      o_bring_shovels_2.done = true
   end)

   while not obj_done(o_bring_shovels_2) do sleep(2029) end
   campaign_message_box(briefing_defeat_pirates)
   o_defeat_pirates = add_campaign_objective(obj_defeat_pirates)

   -- =======================================================================
   -- Allow Hjalmar to build military sites when Runar is defeated.
   -- =======================================================================

   run(function()
      while not p3.defeated do sleep(2089) end
      if not p4.defeated then
         p4:allow_buildings("all")
         p4:forbid_buildings{"barbarians_shipyard","barbarians_weaving_mill","barbarians_scouts_hut"}
      end
   end)

   -- =======================================================================
   -- Check if the pirates have been defeated.
   -- =======================================================================

   run(function()
      while not p5.defeated do sleep(2089) end
      o_defeat_pirates.done = true
      campaign_message_box(briefing_pirates_defeated)
      transport_wares(p1, hq, "gold", 10, true, "tribes/buildings/warehouses/barbarians/headquarters_interim/idle_00.png")
   end)

   -- =======================================================================
   -- Now the swamps have been removed, we have to conquer part of p5.
   -- =======================================================================

   sleep(1200000)

   -- Wait till part of p5 has been conquered.
   while map:get_field(78,65).owner == p5 and
            map:get_field(80,71).owner == p5 and
            map:get_field(80,91).owner == p5 do sleep(2081)
   end
   campaign_message_box(briefing_pirates_regrouping)
   run(function() spawn_soldiers(map:get_field( 87, 65), {0,2,0,2}, 12, 5021) end)   -- HQ p5 I
   run(function() spawn_soldiers(map:get_field(134, 65), {0,2,0,2}, 12, 5023) end)   -- HQ p5 II


   -- Pirates flee.
   while map:get_field(96,65).owner == p5 and
            map:get_field(96,94).owner == p5 and
            map:get_field(87,65).owner == p5 do sleep(2083)
   end
   campaign_message_box(briefing_pirates_on_the_run)

   -- =======================================================================
   -- Conquer the pirate island.
   -- =======================================================================

   sleep(1200000)

   prefilled_buildings(p5,
      { "barbarians_sentry", 80, 27, soldiers = { [{0,3,0,2}] = 2 } },
      { "barbarians_barrier", 82, 29, soldiers = { [{0,3,0,2}] = 3 } },
      { "barbarians_scouts_hut", 80, 30, inputs = {ration = 2} }
   )

   campaign_message_box(briefing_shipyard)
   p1:allow_buildings{"barbarians_shipyard"}

   sleep(30000)
   scroll_to_field(stronghold)
   p1:reveal_fields(stronghold:region(6))
   campaign_message_box(briefing_free_scout)
   o_free_scout = add_campaign_objective(obj_free_scout)
   run(function()
      sleep(30000)
      p1:hide_fields(stronghold:region(6))
   end)
   while map:get_field(80,27).owner == p5 or
            map:get_field(82,29).owner == p5 do sleep(2083)
   end
   o_free_scout.done = true
   campaign_message_box(briefing_island_conquered)
   if immovable_has_name(port_island.immovable, "barbarians_port") then
      add_wares(port_island.immovable, "gold", 14)
      add_wares(port_island.immovable, "ax", 16)
      add_wares(port_island.immovable, "ax_sharp", 12)
      add_wares(port_island.immovable, "ax_broad", 9)
      add_wares(port_island.immovable, "ax_bronze", 7)
      add_wares(port_island.immovable, "ax_battle", 6)
      add_wares(port_island.immovable, "ax_warriors", 4)
      add_wares(port_island.immovable, "helmet", 15)
      add_wares(port_island.immovable, "helmet_mask", 14)
      add_wares(port_island.immovable, "helmet_warhelm", 12)
      add_wares(port_island.immovable, "beer", 24)
      add_wares(port_island.immovable, "beer_strong", 13)
   end

   -- =======================================================================
   -- Pass through swamps towards Hjalmar.
   -- =======================================================================

   sleep(60000)

   p1:reveal_fields(map:get_field(92,50):region(2))
   p1:reveal_fields(map:get_field(91,52):region(2))
   p1:reveal_fields(map:get_field(90,52):region(2))
   p1:reveal_fields(map:get_field(89,54):region(2))
   p1:reveal_fields(map:get_field(89,55):region(2))
   p1:reveal_fields(map:get_field(88,57):region(2))
   p1:reveal_fields(map:get_field(89,59):region(2))
   p1:conquer(map:get_field(91,49),2)
   local f
   f = p1:place_flag(map:get_field(92, 50), true)
   p1:place_road("normal", f, "bl","bl","l","bl","bl","br","bl","bl","br","br", true)
   map:get_field(140,36).terd = "summer_meadow2"
   scroll_to_field(map:get_field(91,49))
   campaign_message_box(briefing_scout)
   o_defeat_green = add_campaign_objective(obj_defeat_green)

   run(function()
      local claimers
      local flag
      while not p4.defeated do
         sleep(7919)
         claimers = map:get_field(91,49).claimers
         if #claimers == 0 and map:get_field(91,49).owner ~= p1 then
            p1:conquer(map:get_field(91,49),2)
            flag = p1:place_flag(map:get_field(92, 50), true)
            p1:place_road("normal", flag, "bl","bl","l","bl","bl","br","bl","bl","br","br", true)
         end
      end
   end)

   run(function()
      sleep(1800000)
      if place_building_if_owner(p4, "barbarians_fortress", map:get_field(102, 56), true) then
         remove_type(map:get_field(103,56), "immovable")
         remove_type(map:get_field(103,55), "immovable")
         remove_type(map:get_field(104,54), "immovable")
         remove_type(map:get_field(104,53), "immovable")
         remove_type(map:get_field(105,52), "immovable")
      end
   end)

   -- =======================================================================
   -- Allow Runar to build military sites when Hjalmar is defeated.
   -- =======================================================================

   run(function()
      while not p4.defeated do sleep(2087) end
      o_defeat_green.done = true
      if not p3.defeated then
         if map:get_field(115,10).owner == p3 then
            p3:conquer(map:get_field(116,19),2)
            p3:place_building("barbarians_tower", map:get_field(116, 19), true, true)
         end
         p3:allow_buildings("all")
         p3:forbid_buildings{"barbarians_shipyard","barbarians_weaving_mill","barbarians_scouts_hut"}
         campaign_message_box(briefing_defeat_yellow)
         o_defeat_yellow = add_campaign_objective(obj_defeat_yellow)
         -- Show barriers on mountain to yellow.
         run(function()
            sleep(2700000)
            p3:reveal_fields(map:get_field(110,84):region(9))
            sleep(300000)
            p3:reveal_fields(map:get_field(101,89):region(9))
            sleep(300000)
            p3:reveal_fields(map:get_field(115,80):region(9))
         end)
         while not p3.defeated do sleep(2087) end
         o_defeat_yellow.done = true
      end
   end)

   while not (p3.defeated and p4.defeated) do sleep(3001) end

   campaign_message_box(briefing_victory)
end

-- =======================================================================

run(mission_thread)
