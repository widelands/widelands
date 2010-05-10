-- =======================================================================
--                      Atlanteans Tutorial Mission 01                      
-- =======================================================================

use("aux", "coroutine")
use("aux", "infrastructure")

-- ===================
-- Constants & Config
-- ===================
set_textdomain("scenario_atl01.wmf")

plr = wl.game.Player(1)
first_tower_field = wl.map.Field(94, 149)
second_tower_field = wl.map.Field(79, 150)

use("map", "texts")

-- =================
-- Helper functions 
-- =================
-- Show one message box
function msg_box(i)
   if i.field then
      scroll_smoothly_to(i.field.trn.trn.trn.trn)

      i.field = nil -- Otherwise message box jumps back
   end

   plr:message_box(i.title, i.body, i)

   sleep(130)
end

-- Show many message boxes
function msg_boxes(boxes_descr)
   for idx,box_descr in ipairs(boxes_descr) do
      msg_box(box_descr)
   end
end


-- ===============
-- Initialization
-- ===============
function initialize()
   plr:allow_buildings("all")

   -- A default headquarters
   use("tribe_atlanteans", "sc00_headquarters_medium")
   init.func(plr) -- defined in sc00_headquarters_medium

   -- Place some buildings
   prefilled_buildings(plr,
      {"high_tower", first_tower_field.x, first_tower_field.y, 
         soldiers = { [{0,0,0,0}] = 1 }
      },
      {"high_tower", second_tower_field.x, second_tower_field.y, 
         soldiers = { [{0,0,0,0}] = 1 }
      }
   )
end

-- ==============
-- Logic Threads 
-- ==============
function intro()
   sleep(1000)

   msg_boxes(initial_messages)
   sleep(200)

   initialize()
end

rise_water = true
step = false
current_water_level = 11
ncoroutines = 0


function water_rise(f, sleeptime)
   -- If we can't do anything yet, put is back to be considered
   -- later
   if f.height > current_water_level then
      if f.terr ~= "wasser" or f.terd ~= "wasser" then
         seeding_fields[#seeding_fields + 1] = f
      end
      return
   end
   ncoroutines = ncoroutines + 1

   while not (f.terr == "wasser" and f.terd == "wasser") do
      if step then
         print(("Caring for %i:%i, n: %i"):format(f.x, f.y, ncoroutines))

         if f.terr ~= "wasser" then
            f.terr = "wasser"
            step = false
            sleep(sleeptime)
         end
         if f.terd ~= "wasser" then
            f.terd = "wasser"
            step = false
            sleep(sleeptime)
         end

         print "Now comes the loop!"

         for idx, nf in ipairs{f.trn, f.tln, f.rn, f.ln, f.brn, f.bln} do
            print "In loop!"
            if nf.terr ~= "wasser" or nf.terd ~= "wasser" then 
               seeding_fields[#seeding_fields + 1] = nf
            end
            print "After if"
         end
      end
      print ("And sleeping!", sleep, sleeptime)
      sleep(sleeptime)
      print "After sleep"
   end
   
   ncoroutines = ncoroutines - 1
end

-- run(intro) TODO: comment me in again

-- TODO: comment me out again
plr.see_all = 1

seeding_fields = {
   wl.map.Field(86,8),
   wl.map.Field(38, 140),
   wl.map.Field(106, 14),
   wl.map.Field(130, 157),
}

function start()
   run(function()
      while true do
         print "2) I woke up!"
         local old_seeding_fields = seeding_fields
         seeding_fields = {}
         while #old_seeding_fields > 0 do 
            local idx = math.random(#old_seeding_fields)
            local f = old_seeding_fields[idx]

            run(function()
               water_rise(f, math.random(300,1000))
            end)
            table.remove(old_seeding_fields, idx)
            print(("%i and %i"):format(#seeding_fields, #old_seeding_fields))
         end
         print "2) I sleep now!"
         sleep(3000)
         print "2) I sleeped now!"
      end
   end)


   run(function()
      while true do
         print "1) I woke up!"
         step = 1
         print "1) I sleep now!"
         sleep(300)
         print "1) I sleeped now!"
      end
   end)
end

