-- =======================================================================
--                            Tutorial Mission
-- =======================================================================

-- ===============
-- Initialization
-- ===============
plr = wl.Game().players[1]
plr:allow_buildings("all")

set_textdomain("Warfare.wmf")

include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/ui.lua"
include "scripting/table.lua"

include "map:scripting/texts.lua"

-- =================
-- Helper functions
-- =================

function _try_add_objective(i)
   -- Add an objective that is defined in the table i to the players objectives.
   -- Returns the new objective or nil. Does nothing if i does not specify an
   -- objective.
   local o = nil
   if i.obj_name then
      o = plr:add_objective(i.obj_name, i.obj_title, i.obj_body)
   end
   return o
end

function msg_box(i)
   local speed = wl.Game().desired_speed
   wl.Game().desired_speed = 1000
   -- this is for slow scrolling
   -- TODO: better write to scroll_smoothly_to

   if i.field then
      scroll_smoothly_to(i.field.trn.trn.trn.trn)

      i.field = nil -- Otherwise message box jumps back
   end

   if i.pos == "topleft" then
      i.posx = 0
      i.posy = 0
   elseif i.pos == "topright" then
      i.posx = 10000
      i.posy = 0
   end

   plr:message_box(i.title, i.body, i)

   local o = _try_add_objective(i)

   sleep(130)
   
   wl.Game().desired_speed = speed

   return o
end

-- ===============
-- initialization
-- ===============

function init_player()

   prefilled_buildings(plr,
      {"fortress", 32, 62, soldiers = {[{3,5,0,2}] = 8 }},
      {"warehouse", 33, 57,
         soldiers = {
            [{0,0,0,0}] = 20, -- needed for training
            [{1,0,0,0}] = 1,
            [{2,0,0,0}] = 1,
            [{3,0,0,0}] = 1,
            [{0,1,0,0}] = 1,
            [{0,2,0,0}] = 1,
            [{0,3,0,0}] = 1,
            [{0,4,0,0}] = 1,
            [{0,5,0,0}] = 1,
            [{0,0,0,1}] = 1,
            [{0,0,0,2}] = 1,
            [{3,5,0,2}] = 30,
         },
         workers = {
            builder = 3,
            ox = 15,
            trainer = 2
         },
         wares = {
            log = 40,
            blackwood = 40,
            cloth = 10,
            gold = 10,
            grout = 30,
            raw_stone = 30,
            thatchreed = 40,
            
            -- wares for training
            sharpax = 20,
            broadax = 20,
            bronzeax = 20,
            battleax = 20,
            warriorsax = 20,
            helm = 20,
            mask = 20,
            warhelm = 20,
            strongbeer = 50,
            pittabread = 200,
            meat = 200
         }
      },
      {"sentry", 28, 57, soldiers = {[{3,5,0,2}] = 2 }},
      {"sentry", 37, 61, soldiers = {[{3,5,0,2}] = 2 }},
      {"barrier", 30, 58, soldier = {[{0,0,0,0}] = 1 }},
      {"lumberjacks_hut", 24, 1}
   )
   
   -- Build the roads
   local map = wl.Game().map
   connected_road(plr,map:get_field(29,58).immovable,"br,r|r,r|r,tr")
   connected_road(plr,map:get_field(38,62).immovable,"l,l|l,bl|tl,tl|tl,tl")
   connected_road(plr,map:get_field(32,63).immovable,"tr,tr|")
   connected_road(plr,map:get_field(25,2).immovable,"tr,tr|tr,tr|tr,tr|tr,tr")

end


-- ================
-- Message threads
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

function test()
create_enemy()
sleep(1000)
msg_box(attack_enemy)
end

run(init_player)
run(intro)
