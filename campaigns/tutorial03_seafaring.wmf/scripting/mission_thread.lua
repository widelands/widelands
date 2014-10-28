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

   while #plr:get_buildings("port") < 2 do sleep(200) end
   o.done = true

   build_ships()
end

function build_ships()
   sleep(200)
   local o = message_box_objective(plr, tell_about_shipyard)
   plr:allow_buildings{"shipyard"}

   while #plr:get_buildings("shipyard") < 1 do sleep(200) end
   o.done = true

   local o = message_box_objective(plr, tell_about_ships)

   -- we cannot check for ships yet (see https://bugs.launchpad.net/widelands/+bug/1380287), so we just wait some time and hope for the best
   sleep(25*60*1000) -- 25 minutes
   o.done = true

   expedition()
end

function expedition()
   sleep(200)
   message_box_objective(plr, expedition1)
   local o = message_box_objective(plr, expedition2)

   -- again, we can only wait. Better a bit too long than too short
   sleep(3*60*1000) -- 3 minutes
   o.done = true

   local o2 = message_box_objective(plr, expedition3)

   while #plr:get_buildings("port") < 3 do sleep(200) end
   o.done = true

   -- places 5 signs with iron to show the player he really found some iron ore
   local fields = map:get_field(97,35):region(3)
   for i=1,5 do
      local successful = false
      while not successful do
         local idx = math.random(#fields)
         f = fields[idx]
         if ((f.resource == "iron") and not f.immovable) then
            map:place_immovable("resi_iron2",f,"atlanteans")
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

