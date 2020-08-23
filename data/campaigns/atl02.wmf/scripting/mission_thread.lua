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


end

function mining()
   mining = add_campaign_objective(obj_mining)
   while not check_for_buildings(p1, {
      atlanteans_coalmine = 1,
      atlanteans_ironmine = 1,
      atlanteans_goldmine = 1
   }) do sleep(3731) end
   mining.done = true
   msg_boxes(mining_established)

end

function spidercloth()
   while count_in_warehouses("spidercloth") > 2 do sleep(2323) end
   msg_boxes(spidercloth_1)
   
   if #p1:get_buildings("atlanteans_farm") < 1 then
      msg_boxes(spidercloth_2)
   end
   local cloth = add_campaign_objective(obj_spidercloth)
   
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
   msg_boxes(enemy_1)
   while not check_for_buildings(p1, {
      atlanteans_scouts_house = 1,
   }) do sleep(3731) end
   local scout = nil
   local contact = nil
   while not scout do
      -- let's see if a scout is going around
      for i,house in pairs(p1:get_buildings("atlanteans_scouts_house")) do
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
   while not (scout.field.immovable and scout.field.immovable.descr.name == "atlanteans_scouts_house") do
      sleep (2000)
   end
   
   set_objective_done(explore)
   msg_boxes(allies)
   run(allies)
   run(maletus_defeated)
   run(check_defeat)
end

function allies()
   p1:set_attack_forbidden(3, true)
   Kalitath:set_attack_forbidden(1, true)
   sleep(300000)
   msg_boxes(tribute)
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
   sleep(500)

   p1.see_all = true -- TODO: remove this
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

run(intro)

