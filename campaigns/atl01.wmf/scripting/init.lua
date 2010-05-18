-- =======================================================================
--                      Atlanteans Tutorial Mission 01                      
-- =======================================================================

use("aux", "coroutine")
use("aux", "infrastructure")

use("map", "water_rising")

-- TODO: document Set class
-- TODO: remove table_unique 

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


function rise_water()

   local seeding_fields = {
      wl.map.Field(86,8),
      wl.map.Field(38, 140),
      wl.map.Field(106, 14),
      wl.map.Field(130, 157),
   }
   
   -- Global variable!
   wr = WaterRiser:new(seeding_fields)

   wr:run()
end

-- TODO: comment me out again
plr.see_all = 1

-- run(intro) TODO: comment me in again

