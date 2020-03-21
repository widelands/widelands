-- ===============
-- Mission thread
-- ===============

local waterways_done = false
local expedition_done = false
local gold_mine_done = false
local ferries_done = false

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
   sleep(2000)
   message_box_objective(plr, tell_about_port)

   wl.ui.MapView().buildhelp = true -- so that the player sees the port building icon
   local o = message_box_objective(plr, tell_about_port_building)

   while not construction_started(second_port_field, "atlanteans_port") do sleep(1000) end
   run(build_ships)

   while #plr:get_buildings("atlanteans_port") < 2 do sleep(1000) end
   set_objective_done(o)
end

function watch_ship_production()
   -- warn player about over-producing
   while #plr:get_ships() < 2 do sleep(30*1000) end
   local ship2 = plr:get_ships()[2]
   scroll_to_field(ship2.field)
   message_box_objective(plr, shipyard_production(ship2.shipname))
end

function build_ships()
   sleep(20*1000)
   local o = message_box_objective(plr, tell_about_shipyard)
   plr:allow_buildings{"atlanteans_shipyard"}

   local area = home_bay:region(20)
   while not construction_started_region(area, "atlanteans_shipyard") do sleep(1000) end
   run(waterways)

   while #plr:get_buildings("atlanteans_shipyard") < 1 do sleep(1000) end
   set_objective_done(o)

   local o = message_box_objective(plr, tell_about_ships)

   -- we only wait for one ship and a bit longer because it takes long enough
   while #plr:get_ships() < 1 do sleep(30*1000) end
   run(watch_ship_production)
   sleep(3*60*1000)

   set_objective_done(o)
   expedition()
end

function expedition()
   sleep(2000)
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
   local fields = iron_on_island:region(3)
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

   conclude_expedition()
end

function conclude_expedition()
   additional_port_space.terr = "desert_steppe" -- make it land again so that the player can build a port
   message_box_objective(plr, expedition4)
   sleep(3000)
   iron_mine()
end

function iron_mine()
   local o = message_box_objective(plr, expedition5)
   while #plr:get_buildings("atlanteans_ironmine") < 1 do sleep(3000) end
   print(string.format("get_buildings('atlanteans_ironmine') = %d", #plr:get_buildings("atlanteans_ironmine")))
   set_objective_done(o)
   expedition_done = true
end

function complete_ferry_yard()
   plr:allow_buildings{"atlanteans_ferry_yard"}
   local o = message_box_objective(plr, ferry_2)
   while #plr:get_buildings("atlanteans_ferry_yard") < 1 do sleep(3000) end
   set_objective_done(o)
end

function complete_gold_mine()
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
   gold_mine_done = true
end

function complete_ferries()
   -- wait until 4 ferries are assigned to waterways
   while plr:get_workers("atlanteans_ferry") < 4 do sleep(3000) end
   ferries_done = true
end

function watch_ferry_production()
   -- warn player about over-producing
   while plr:get_workers("atlanteans_ferry") < 1 do sleep(3000) end
   message_box_objective(plr, ferry_yard_production)
end

function waterways()
   sleep(5000)
   map:place_immovable("atlanteans_resi_gold_2", gold_mine, "tribes")

   message_box_objective(plr, ferry_1)
   sleep(1000)

   run(complete_ferry_yard)
   local area = shore:region(37)
   while not construction_started_region(area, "atlanteans_ferry_yard") do sleep(1000) end
   sleep(5000)
   -- place resource indicator again in case player preferred ships/exploring
   map:place_immovable("atlanteans_resi_gold_2", gold_mine, "tribes")

   message_box_objective(plr, ferry_3)
   sleep(500)
   message_box_objective(plr, ferry_4)

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
   -- check for goldmine, and waterways with ferries
   run(complete_gold_mine)
   run(complete_ferries)
   watch_ferry_production()
   while not (gold_mine_done and ferries_done) do sleep(3000) end
   set_objective_done(o)

   message_box_objective(plr, ferry_6)
   waterways_done = true
end

function conclude_tutorial()
   while not (expedition_done and waterways_done) do sleep(3000) end
   message_box_objective(plr, congratulation)
end

run(introduction)
run(conclude_tutorial)
