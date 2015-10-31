-- ===============
-- Mission thread
-- ===============

function introduction()
   additional_port_space.terr = "wasser" -- disable the port space
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
   o.done = true

   build_ships()
end

function build_ships()
   sleep(200)
   local o = message_box_objective(plr, tell_about_shipyard)
   plr:allow_buildings{"atlanteans_shipyard"}

   while #plr:get_buildings("atlanteans_shipyard") < 1 do sleep(200) end
   o.done = true

   local o = message_box_objective(plr, tell_about_ships)

   -- we only wait for one ship and a bit longer because it takes long enough
   while #plr:get_ships() < 1 do sleep(30*1000) end
   sleep(5*60*1000)

   o.done = true

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
   o.done = true

   local o2 = message_box_objective(plr, expedition3)

   while #plr:get_buildings("atlanteans_port") < 3 do sleep(200) end
   o.done = true

   -- places 5 signs with iron to show the player he really found some iron ore
   local fields = map:get_field(97,35):region(3)
   for i=1,5 do
      local successful = false
      while not successful do
         local idx = math.random(#fields)
         f = fields[idx]
         if ((f.resource == "iron") and not f.immovable) then
            map:place_immovable("resi_iron2",f,"tribes")
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
end

run(introduction)