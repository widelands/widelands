-- ================
-- Mission thread
-- ================

function intro()
   reveal_concentric(plr, wl.Game().map:get_field(32, 59), 15)
   sleep(1000)
   message_box_objective(plr, introduction)

   training()
end

local trainingcamp_done = false
local battlearena_done = false

function training2()
   -- Teach about trainingsites and soldiers' abilities - concurrent part 2
   sleep(2*60*1000)
   o = message_box_objective(plr, trainingcamp1)
   while #plr:get_buildings("barbarians_trainingcamp") == 0 do sleep(500) end
   set_objective_done(o)
   message_box_objective(plr, trainingcamp2)
   trainingcamp_done = true
end

function training()
   -- Teach about trainingsites and soldiers' abilities
   sleep(5000) -- to let soldiers walk

   message_box_objective(plr, abilities)
   local o = message_box_objective(plr, battlearena1)
   run(training2)

   while #plr:get_buildings("barbarians_battlearena") == 0 do sleep(500) end
   set_objective_done(o, 0)
   message_box_objective(plr, battlearena2)
   battlearena_done = true
end

function military_buildings()
   while not trainingcamp_done or not battlearena_done do sleep(3000) end
   message_box_objective(plr, heroes_rookies)
   message_box_objective(plr, soldier_capacity)
   local o = message_box_objective(plr, dismantle)

   while #plr:get_buildings("barbarians_sentry") > 1 do sleep(200) end
   set_objective_done(o)

   enhance_fortress()
end

function enhance_fortress()
   sleep(5000)

   local citadel_field = wl.Game().map:get_field(32, 62)
   local o = message_box_objective(plr, fortress_enhancement)
   while not (citadel_field.immovable and
      citadel_field.immovable.descr.name == "barbarians_citadel") do sleep(800) end
   set_objective_done(o, 0)

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
   local map = wl.Game().map
   local p2 = wl.Game().players[2]
   prefilled_buildings(p2,
      {"empire_barrier", 24, 7},
      {"empire_sentry", 29, 16},
      {"empire_tower", 30, 21},
      {"empire_headquarters", 30, 27,
         soldiers = {
            [{0,0,0,0}] = 15,
         }
      }
   )
   connected_road(p2,map:get_field(29,17).immovable,"tr,tl|tl,tl|tl,tl|tl,tl|tl,l")
   connected_road(p2,map:get_field(31,22).immovable,"tr,tl|tl,tl,tl")
   connected_road(p2,map:get_field(31,28).immovable,"tr,tr|tr,tl|tl,tl")
   p2:forbid_buildings("all")
end

function attack()
   local o = message_box_objective(plr, attack_enemy)

   local plr2 = wl.Game().players[2]
   while #plr2:get_buildings("empire_headquarters") > 0 do
      sleep(3000)
   end
   set_objective_done(o)

   conclusion()
end

function conclusion()
   message_box_objective(plr, conclude_tutorial)
end

run(intro)
run (military_buildings)
