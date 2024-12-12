-- ================
-- Mission thread
-- ================

function intro()
   reveal_concentric(plr, sf, 15)
   sleep(1000)
   campaign_message_box(introduction)

   training()
end

local trainingcamp_done = false
local battlearena_done = false
local scouting_done = false

function training2()
   -- Teach about trainingsites and soldiers' abilities - concurrent part 2
   sleep(60*1000)
   local o = campaign_message_with_objective(trainingcamp1, obj_trainingcamp)
   while #plr:get_buildings("barbarians_trainingcamp") == 0 do sleep(500) end
   set_objective_done(o)
   campaign_message_box(trainingcamp2)
   trainingcamp_done = true
end

function scouting()
   -- Teach player about scouting
   sleep(2*60*1000)
   local o = campaign_message_with_objective(scouting1, obj_scouting1)
   while #plr:get_buildings("barbarians_scouts_hut") == 0 do sleep(500) end
   set_objective_done(o)

   exploring()
end

function exploring()
   local pois = sf:region(30, 29)
   local o = campaign_message_with_objective(scouting2, obj_scouting2)
   while not any_field_seen(plr, pois) do sleep(2000) end
   set_objective_done(o)
   campaign_message_box(scouting3)
   scouting_done = true
end

function training()
   -- Teach about trainingsites and soldiers' abilities
   sleep(5000) -- to let soldiers walk

   campaign_message_box(abilities)
   local o = campaign_message_with_objective(battlearena1, obj_battlearena)
   run(training2)
   run(scouting)

   while #plr:get_buildings("barbarians_battlearena") == 0 do sleep(500) end
   set_objective_done(o, 0)
   campaign_message_box(battlearena2)
   battlearena_done = true
end

function military_buildings()
   while not trainingcamp_done or not battlearena_done or
      not scouting_done do sleep(3000) end
   campaign_message_box(heroes_rookies)
   campaign_message_box(soldier_capacity)
   campaign_message_box(hq_soldiers)
   campaign_message_box(hq_status)

   local o = campaign_message_with_objective(dismantle, obj_dismantle)

   while #plr:get_buildings("barbarians_sentry") > 1 do sleep(200) end
   set_objective_done(o)

   enhance_fortress()
end

function enhance_fortress()
   sleep(5000)

   local citadel_field = wl.Game().map:get_field(32, 62)
   local o = campaign_message_with_objective(fortress_enhancement, obj_fortress)
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
   connected_road("normal", p2,map:get_field(29,17).immovable,"tr,tl|tl,tl|tl,tl|tl,tl|tl,l")
   connected_road("normal", p2,map:get_field(31,22).immovable,"tr,tl|tl,tl,tl")
   connected_road("normal", p2,map:get_field(31,28).immovable,"tr,tr|tr,tl|tl,tl")
   p2:forbid_buildings("all")
end

function attack()
   local o = campaign_message_with_objective(attack_enemy, obj_attack)

   local plr2 = wl.Game().players[2]
   while #plr2:get_buildings("empire_headquarters") > 0 do
      sleep(3000)
   end
   set_objective_done(o)

   conclusion()
end

function conclusion()
   campaign_message_box(conclude_tutorial)
end

run(intro)
run(military_buildings)
