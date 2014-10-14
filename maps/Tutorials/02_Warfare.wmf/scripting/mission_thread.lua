-- ================
-- Mission thread
-- ================

function intro()
   sleep(200)
   msg_box(introduction)

   training()
end

function training()
   -- Teach about trainingsites and soldiers' abilities
   sleep(5000) -- to let soldiers walk

   msg_box(abilities)
   local o = msg_box(battlearena1)
   while #plr:get_buildings("battlearena") == 0 do sleep(500) end
   o.done = true
   msg_box(battlearena2)

   o = msg_box(trainingcamp1)
   while #plr:get_buildings("trainingcamp") == 0 do sleep(500) end
   o.done = true
   msg_box(trainingcamp2)

   sleep(300)

   military_buildings()
end

function military_buildings()
   msg_box(heroes_rookies)
   msg_box(soldier_capacity)
   local o = msg_box(dismantle)

   while #plr:get_buildings("sentry") > 1 do sleep(200) end
   o.done = true

   sleep(2000)

   enhance_fortress()
end

function enhance_fortress()
   sleep(5000)

   local citadel_field = wl.Game().map:get_field(32, 62)
   local o = msg_box(fortress_enhancement)
   while not (citadel_field.immovable and
      citadel_field.immovable.descr.name == "citadel") do sleep(800) end
   o.done = true

   create_enemy()

   -- Wait for soldiers to move in
   local citadel = citadel_field.immovable
   local break_out = false
   while not break_out do
      for k,v in pairs(citadel:get_soldiers("all")) do
         break_out = true
         break -- Break out if there is at least one soldier here
      end

      sleep(500)
   end
   
   sleep(300)

   attack()
end

function create_enemy()
   prefilled_buildings(wl.Game().players[2],
      {"barrier", 24, 7},
      {"sentry", 29, 16},
      {"tower", 30, 21},
      {"headquarters", 30, 27,
         soldiers = {
            [{0,0,0,0}] = 15,
         }
      }
   )
   wl.Game().players[2]:forbid_buildings("all")
end

function attack()
   local o = msg_box(attack_enemy)

   local plr2 = wl.Game().players[2]
   while #plr2:get_buildings("headquarters") > 0 do
      sleep(3000)
   end
   o.done = true

   conclusion()
end

function conclusion()
   sleep(4000)
   msg_box(conclude_tutorial)
end


run(intro)
