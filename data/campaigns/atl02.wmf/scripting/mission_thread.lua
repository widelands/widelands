-- =======================================================================
--                      Atlanteans Tutorial Mission 02
-- =======================================================================

-- ==============
-- Logic Threads
-- ==============

function iron()
   while not check_for_buildings(p1, {
      atlanteans_smelting_works = 1,
      atlanteans_toolsmithy = 1,
   }) do sleep(3731) end
   tools.done = true
   msg_boxes(tools_available(leader))
   run(mining)
   --Kalitath:set_attack_forbidden(2, false)
   --Maletus:set_attack_forbidden(3, false)
end

function mining()
   mining = add_campaign_objective(obj_mining)
   while not check_for_buildings(p1, {
      atlanteans_coalmine = 1,
      atlanteans_ironmine = 1,
      atlanteans_goldmine = 1
   }) do sleep(3731) end
   mining.done = true
   msg_boxes(mining_established(leader))
end

function spidercloth()
   sleep(300000) -- sleep until the second ship is unloaded
   while count_in_warehouses("spidercloth") > 2 do sleep(2323) end
   local cloth = add_campaign_objective(obj_spidercloth)
   msg_boxes(spidercloth_1(leader))

   if #p1:get_buildings("atlanteans_farm") < 1 then
      msg_boxes(spidercloth_2(leader))
   end

   while not check_for_buildings(p1, {
      atlanteans_farm = 1,
      atlanteans_spiderfarm = 1,
      atlanteans_weaving_mill = 1,
   }) do sleep(3731) end
   cloth.done = true
   msg_boxes(spidercloth_3(leader))
end

function enemy()
   local seen = nil
   while not seen do
      sleep(4000)
      for x=40, 70 do
         for y=120, 160 do
            local field = map:get_field(x,y)
            if field.owner == Kalitath and p1:sees_field(field) then
               seen = true
            end
         end
      end
   end

   if check_for_buildings(p1, {atlanteans_scouts_house1 = 1,}) then
      msg_boxes(enemy_1)
   else
      msg_boxes(enemy_1a)
   end
end

function expansion()
   while not check_for_buildings(p1, {
      atlanteans_tower = 1,
      atlanteans_guardhouse = 2,
   }) do sleep(3731) end
   expand.done = true
end

function scouting()
   while not check_for_buildings(p1, {
      atlanteans_scouts_house1 = 1,
   }) do sleep(3731) end
   Kalitath:set_attack_forbidden(2, false)
   Maletus:set_attack_forbidden(3, false)
   local scout = nil
   local contact = nil
   while not scout do
      sleep(2000)
      -- let's see if a scout is going around
      for i,house in pairs(p1:get_buildings("atlanteans_scouts_house1")) do
         for j,field in pairs(house.fields[1]:region(17)) do -- the scout has a radius of 15
            for k,bob in pairs(field.bobs) do
               if bob.descr.name == "atlanteans_scout" then
                  scout = bob
                  break
               end
            end
         end
      end
   end
   while not contact do
      sleep(2000)
      if scout.field.owner == Kalitath and
         scout.field.brn.owner == Kalitath and
         scout.field.bln.owner == Kalitath and
         scout.field.trn.owner == Kalitath and
         scout.field.tln.owner == Kalitath and
         scout.field.rn.owner == Kalitath and
         scout.field.ln.owner == Kalitath then
         contact = true
      end
   end
   while not (scout.field.immovable and scout.field.immovable.descr.name == "atlanteans_scouts_house1") do
      sleep (2000)
   end

   explore.done = true
   msg_boxes(allies)
   run(uncertain_allies)
   run(maletus_defeated)
end

function uncertain_allies()
   p1:set_attack_forbidden(3, true)
   run(check_kalitath_defeated)
   sleep(60000)
   msg_boxes(tribute)
   Kalitath:set_attack_forbidden(1, true)
   leader = 2
   trade = add_campaign_objective(obj_tribute)
   run(patience)
   reveal_concentric(p1, map:get_field(112,150), 5, 500)
   trading_post = add_campaign_objective(obj_trading_post)
   while not check_for_buildings(p1, {
      atlanteans_trading_post = 1,
   }) do sleep(3731) end
   trading_post.done = true
   msg_boxes(trading)
   run(wares_delivery)
end

function wares_delivery()
   local coal = 0
   local iron = 0
   local gold = 0
   local wood = 0

   while not Kalitath.defeated do
      sleep(5000)
      local hq = Kalitath:get_buildings("barbarians_headquarters")
      local delivered_wood = p1:get_produced_wares_count("coin_wood")
      if (delivered_wood > wood) then
         if hq and hq[1] then
            local amount = hq[1]:get_wares("log") + 5
            hq[1]:set_wares("log", amount)
            wood = delivered_wood
         end
      end
      local delivered_coal = p1:get_produced_wares_count("coin_copper")
      if (delivered_coal > coal) then
         if hq and hq[1] then
            local amount = hq[1]:get_wares("coal") + 4
            hq[1]:set_wares("coal", amount)
            coal = delivered_coal
         end
      end
      local delivered_iron = p1:get_produced_wares_count("coin_silver")
      if (delivered_iron > iron) then
         if hq and hq[1] then
            local amount = hq[1]:get_wares("iron") + 3
            hq[1]:set_wares("iron", amount)
            iron = delivered_iron
         end
      end
      local delivered_gold = p1:get_produced_wares_count("coin_gold")
      if (delivered_gold > gold) then
         if hq and hq[1] then
            local amount = hq[1]:get_wares("gold") + 2
            hq[1]:set_wares("gold", amount)
            gold = delivered_gold
         end
      end
   end
end

function patience()
   local trade_started = false
   local count = 0
   local penalty = 1
   while not (trade_started or p1.defeated or Kalitath.defeated) do
      if (p1:get_produced_wares_count("coin_wood") + p1:get_produced_wares_count("coin_copper") + p1:get_produced_wares_count("coin_silver") + p1:get_produced_wares_count("coin_gold")) > 0 then
         trade_started = true
      end
      count = count + 1
      if count == 3601 then
         trade.done = true
         msg_boxes(tribute_not_started)
         trade = add_campaign_objective(obj_tribute2)
         penalty = 2
         local port = p1:get_buildings("atlanteans_port")
         if port and port[1] then
            port[1].flag.ware_economy:set_target_quantity("coin_wood", 5 * penalty)
            port[1].flag.ware_economy:set_target_quantity("coin_copper", 4 * penalty)
            port[1].flag.ware_economy:set_target_quantity("coin_silver", 3 * penalty)
            port[1].flag.ware_economy:set_target_quantity("coin_gold", 2 * penalty)
         end
      end
      if count == 6001 then
         msg_boxes(alliance_broken)
         msg_boxes(suicide)
         wl.ui.MapView():close()
      end
      sleep(500)
   end
   msg_boxes(tribute_started)
   defeat_maletus = add_campaign_objective(obj_defeat_maletus)
   local end_time = game.time + 12600000 -- 3,5 hours until timeout
   while not (p1:get_produced_wares_count("coin_wood") >= penalty * 5 and p1:get_produced_wares_count("coin_copper") >= penalty * 4 and p1:get_produced_wares_count("coin_silver") >= penalty * 3 and p1:get_produced_wares_count("coin_gold") >= penalty * 2) do
      sleep(4235)
      if game.time > end_time then
         msg_boxes(alliance_broken_1)
         msg_boxes(suicide)
         wl.ui.MapView():close()
      end
   end
   trade.done = true
   if defeat_maletus.done == false then
      msg_boxes(maletus_defeated_1a)
   end
end

function maletus_defeated()
   while not Maletus.defeated do sleep(6000) end
   defeat_maletus.done = true
   if trade.done == true then
      msg_boxes(maletus_defeated_1)
   else
      msg_boxes(maletus_defeated_2a)
      while not trade.done == true do
         sleep(1333)
      end
      msg_boxes(maletus_defeated_2)
   end
   msg_boxes(maletus_defeated_3)
   Kalitath:set_attack_forbidden(1, false)
   sleep(5000)
   p1:allow_buildings{"atlanteans_temple_of_satul"}
   msg_boxes(heretics_again)
   temple = add_campaign_objective(obj_build_temple)
   run(final)
end

function final()
   while not check_for_buildings(p1, {
      atlanteans_temple_of_satul = 1,
      atlanteans_scouts_house1 = 1,
   })  do sleep(3731) end
   temple.done = true
   msg_boxes(temple_completed)
   priestesses = add_campaign_objective(obj_worship)
   while count_workers_in_warehouses("atlanteans_priestess") < 2 do sleep(2323) end
   priestesses.done = true
   msg_boxes(priestesses_consecrated)
   sleep(2000)
   run(volcano_eruptions)
   sleep(30000)
   msg_boxes(princess_back)
   p1:set_attack_forbidden(3, false)
   defeat_kalitath = add_campaign_objective(obj_defeat_kalitath)
   while not Kalitath.defeated do sleep(6000) end
   defeat_kalitath.done = true
   msg_boxes(victory)
end

function infrastructure()
   while not check_for_buildings(p1, {
      atlanteans_woodcutters_house = 3,
      atlanteans_quarry = 1,
      atlanteans_sawmill = 1,
   }) do sleep(3731) end
   basic_infrastructure.done = true
   msg_boxes(infrastructure_1(leader))
end

function intro()
   Kalitath:set_attack_forbidden(2, true)
   Maletus:set_attack_forbidden(3, true)
   sleep(500)

   --p1.see_all = true -- TODO: remove this
   msg_boxes(initial_messages)
   include "map:scripting/starting_conditions.lua"

   local port = add_campaign_objective(obj_find_port)
   while not check_for_buildings(p1, {
      atlanteans_port = 1,
   }) do sleep(3731) end
   port.done = true
   basic_infrastructure = add_campaign_objective(obj_basic_infrastructure)
   tools = add_campaign_objective(obj_tools)
   explore = add_campaign_objective(obj_explore)
   expand = add_campaign_objective(obj_expand)
   msg_boxes(initial_messages1)

   run(spidercloth)
   run(infrastructure)
   run(enemy)
   run(expansion)
   run(scouting)
   run(iron)
   run(check_defeat)
end

function check_defeat()
   while not p1.defeated do sleep(6000) end
   msg_boxes(defeated)
   wl.ui.MapView():close()
end

function check_kalitath_defeated()
   while not (p1.defeated or Maletus.defeated) do
      if Kalitath.defeated then
         msg_boxes(kalitath_dead)
         wl.ui.MapView():close()
      end
      sleep(6000)
   end
end

run(intro)
