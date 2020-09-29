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
   msg_boxes(tools_available)
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
   if explore.done == true then
      msg_boxes(mining_established_2)
   else
      msg_boxes(mining_established_1)
   end
end

function spidercloth()
   sleep(300000) -- sleep until the second ship is unloaded
   while count_in_warehouses("spidercloth") > 2 do sleep(2323) end
   local cloth = add_campaign_objective(obj_spidercloth)
   msg_boxes(spidercloth_1)

   if #p1:get_buildings("atlanteans_farm") < 1 then
      msg_boxes(spidercloth_2)
   end

   while not check_for_buildings(p1, {
      atlanteans_farm = 1,
      atlanteans_spiderfarm = 1,
      atlanteans_weaving_mill = 1,
   }) do sleep(3731) end
   cloth.done = true
   msg_boxes(spidercloth_3)
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

   Kalitath:set_attack_forbidden(2, false)
   Maletus:set_attack_forbidden(3, false)

   msg_boxes(enemy_1)
   while not check_for_buildings(p1, {
      atlanteans_scouts_house1 = 1,
   }) do sleep(3731) end
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

   set_objective_done(explore)
   msg_boxes(allies)
   run(uncertain_allies)
   run(maletus_defeated)
   run(check_defeat)
end

function uncertain_allies()
   p1:set_attack_forbidden(3, true)
   Kalitath:set_attack_forbidden(1, true)
   sleep(60000)
   msg_boxes(tribute)
   reveal_concentric(p1, map:get_field(112,150), 5, 500)
   trading_post = add_campaign_objective(obj_trading_post)
   while not check_for_buildings(p1, {
      atlanteans_trading_post = 1,
   }) do sleep(3731) end
   trading_post.done = true
   trade = add_campaign_objective(obj_tribute)
   run(check_kalitath_defeated)
   msg_boxes(trading)
   run(patience)

end

function patience()
   local trade_started = false
   local count = 0
   while not (trade_started or p1.defeated or kalitath.defeated) do
      if (p1:get_produced_wares_count("coin_wood") + p1:get_produced_wares_count("coin_copper") + p1:get_produced_wares_count("coin_silver") + p1:get_produced_wares_count("coin_gold")) > 0 then
         trade_started = true
      end
      count = count + 1
      if count == 2401 then
         msg_boxes(tribute_not_started)
      end
      sleep(500)
   end
end

function maletus_defeated()
   while not Maletus.defeated do sleep(6000) end
   msg_boxes(maletus_defeated)
end

function infrastructure()
   while not check_for_buildings(p1, {
      atlanteans_woodcutters_house = 3,
      atlanteans_quarry = 1,
      atlanteans_sawmill = 1,
   }) do sleep(3731) end
   basic_infrastructure.done = true
   msg_boxes(infrastructure_1)
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
   msg_boxes(initial_messages1)

   run(spidercloth)
   run(infrastructure)
   run(enemy)
   run(iron)

end

function check_defeat()
   while not p1.defeated do sleep(6000) end
   msg_boxes(defeated)
   p1.see_all = true
end

function check_kalitath_defeated()
   defeat = p1.defeated == true
   while not defeat or trade.done ==true do
      sleep(6000)
      defeat = p1.defeated == true
      if kalitath.defeated then
         defeat = true
         msg_boxes(kalitath_dead)
      end
   end
   if defeat == true then
      msg_boxes(defeated)
      p1.see_all = true
   end
end

run(intro)
