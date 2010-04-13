-- =======================================================================
--                       Barbarians Campaign Mission 1
-- =======================================================================

set_textdomain("tutorial01.wmf")

-- ===============
-- Initialization
-- ===============
plr = wl.game.Player(1)
plr:allow_buildings("all")

-- A default headquarters
use("tribe_barbarians", "sc_headquarters_medium")
initialize(plr)


use("aux", "coroutine")
use("aux", "smooth_move")
use("aux", "table")

-- Constants
first_lumberjack_field = wl.map.Field(15,11)
use("map", "texts")

function msg_box(i)

   if i.field then
      smooth_move(i.field.trn.trn.trn.trn)
      sleep(2000)
      i.field = nil -- Otherwise message box jumps back
   end

   if i.pos == "topleft" then
      i.posx = 0
      i.posy = 0
   elseif i.pos == "topright" then
      i.posx = 10000
      i.posy = 0
   end

   wl.game.Player(1):message_box(i.title, i.body, i)

   sleep(130)
end

function starting_infos()
   sleep(100)

   msg_box(initial_message_01)
   msg_box(initial_message_02)

   -- Wait for buildhelp to come on
   while not plr.buildhelp do
      sleep(200)
   end
   sleep(1200)

   build_lumberjack()
end

function build_lumberjack()
   sleep(100)

   msg_box(lumberjack_message_01)
   msg_box(lumberjack_message_02)

   plr:place_building("lumberjacks_hut", first_lumberjack_field, true)
   msg_box(lumberjack_message_03)

   plr:place_road(first_lumberjack_field.brn.immovable, "l", "tl", "l", "l")
   msg_box(lumberjack_message_04)
   sleep(15000) 

   msg_box(lumberjack_message_05)

   -- Wait for flag
   local f = wl.map.Field(14,11)
   while not (f.immovable and f.immovable.type == "flag") do sleep(300) end

   msg_box(lumberjack_message_06)

   while #plr:get_buildings("lumberjacks_hut") < 1 do sleep(300) end
   
   msg_box(lumberjack_message_07)
end

-- run(starting_infos)
run(build_lumberjack)

