-- ===============
-- Mission thread
-- ===============

function introduction()
   fields = get_sees_fields(plr)
   reveal_randomly(plr, fields, 2000)
   additional_port_space.terr = "summer_water" -- disable the port space
   sleep(1000)
   message_box_objective(plr, intro_south)
   sleep(300)
   message_box_objective(plr, intro_north)
   sleep(20000)

   build_port()
end

function build_port()
   sleep(200)
   message_box_objective(plr, tell_about_port)

   wl.ui.MapView().buildhelp = true -- so that the player sees the port building icon
   local o = message_box_objective(plr, tell_about_port_building)

   while #plr:get_buildings("atlanteans_port") < 2 do sleep(200) end
   set_objective_done(o)

   build_ships()
end

function build_ships()
   sleep(200)
   local o = message_box_objective(plr, tell_about_shipyard)
   plr:allow_buildings{"atlanteans_shipyard"}

   while #plr:get_buildings("atlanteans_shipyard") < 1 do sleep(200) end
   set_objective_done(o)

   local o = message_box_objective(plr, tell_about_ships)

   -- we only wait for one ship and a bit longer because it takes long enough
   while #plr:get_ships() < 1 do sleep(30*1000) end
   sleep(5*60*1000)

   set_objective_done(o)

   expedition()
end

function expedition()
   sleep(200)
   message_box_objective(plr, expedition1)
   local o = message_box_objective(plr, expedition2)

   local function _ship_ready_for_expedition()
      for k,ship in ipairs(plr:get_ships()) do
         if ship.state == "exp_waiting" then return true end
      end
      return false
   end

   while not _ship_ready_for_expedition() do sleep(1000) end
   set_objective_done(o)

   o = message_box_objective(plr, expedition3)

   while #plr:get_buildings("atlanteans_port") < 3 do sleep(200) end
   set_objective_done(o)

   -- places 5 signs with iron to show the player he really found some iron ore
   local fields = map:get_field(97,35):region(3)
   for i=1,5 do
      local successful = false
      while not successful do
         local idx = math.random(#fields)
         f = fields[idx]
         if ((f.resource == "iron") and not f.immovable) then
            map:place_immovable("atlanteans_resi_iron_2",f,"tribes")
            successful = true
         end
         table.remove(fields,idx)
      end
   end

   conclude()
end

function conclude()
   additional_port_space.terr = "desert_steppe" -- make it land again so that the player can build a port
   message_box_objective(plr, conclusion)
   sleep(5000)
   waterways()
end

function waterways()
   map:place_immovable("atlanteans_resi_gold_2", map:get_field(23, 102), "tribes")
   message_box_objective(plr, ferry_1)
   sleep(1000)
   message_box_objective(plr, ferry_2)
   sleep(500)
   message_box_objective(plr, ferry_3)
   sleep(500)
   message_box_objective(plr, ferry_4)
   plr:allow_buildings{"atlanteans_ferry_yard"}

   -- Build waterway
   click_on_field(waterway_field)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.build_waterway)
   click_on_field(waterway_field.trn)
   click_on_field(waterway_field.trn.trn)
   click_on_field(waterway_field.trn.trn.tln)
   sleep(1000)
   click_on_field(waterway_field.trn)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.destroy_waterway)

   local o = message_box_objective(plr, ferry_5)
   while #plr:get_buildings("atlanteans_ferry_yard") < 1 do sleep(2500) end
   -- check for goldmine, and waterways with ferries
   local field_for_mine = map:get_field(20, 102):region(5)
   while field_for_mine do
      sleep(3000)
      for i,f in pairs(field_for_mine) do
         if f.immovable and f.immovable.descr.name == "atlanteans_goldmine" then
            field_for_mine = nil
            break
         end
      end
   end
   local waterways = {}
   while #waterways < 4 do
      for x = 13, 51 do
         sleep(250)
         for y = 67, 100 do
            local f = map:get_field(x, y)
            if f.immovable and f.immovable.descr.type_name == "waterway" and
                  f.immovable:get_workers("atlanteans_ferry") > 0 then
               local ww = f.immovable
               for i,w in pairs(waterways) do
                  if w == ww then
                     ww = nil
                     break
                  end
               end
               if ww then table.insert(waterways, ww) end
            end
         end
      end
   end

   set_objective_done(o)
   message_box_objective(plr, ferry_6)
   sleep(1000)
   message_box_objective(plr, ferry_7)
end

run(introduction)
