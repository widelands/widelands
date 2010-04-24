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
use("tribe_barbarians", "sc00_headquarters_medium")
init.func(plr) -- defined in sc00_headquarters_medium


use("aux", "coroutine")
use("aux", "smooth_move")
use("aux", "table")

-- Constants
first_lumberjack_field = wl.map.Field(15,11)
first_quarry_field = wl.map.Field(8,12)
conquer_field = wl.map.Field(19,15)

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

   plr:message_box(i.title, i.body, i)

   sleep(130)
end

function send_message(i)
   plr:send_message(i.title, i.body, i)

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

   -- msg_box(lumberjack_message_01)
   -- msg_box(lumberjack_message_02)

   mv = wl.ui.MapView()
   mv:click(first_lumberjack_field)
   
   sleep(1500)
   print ("Now I click on the lumberjack!")
   sleep(1500)

   mv.windows.field_action:close()
   for name,w in pairs(mv.windows) do print ("## ", name) end

   plr:place_building("lumberjacks_hut", first_lumberjack_field, true)
   -- msg_box(lumberjack_message_03)
   --
   -- plr:place_road(first_lumberjack_field.brn.immovable, "l", "tl", "l", "l")
   -- msg_box(lumberjack_message_04)
   -- sleep(15000)
   --
   -- msg_box(lumberjack_message_05)
   --
   -- -- Wait for flag
   -- local f = wl.map.Field(14,11)
   -- while not (f.immovable and f.immovable.type == "flag") do sleep(300) end
   --
   -- msg_box(lumberjack_message_06)
   --
   -- while #plr:get_buildings("lumberjacks_hut") < 1 do sleep(300) end
   --
   -- msg_box(lumberjack_message_07)
   --
   -- learn_to_move()
end

function learn_to_move()
   -- Teaching the user how to scroll on the map
   msg_box(inform_about_stones)
   
   function _wait_for_move()
      local cx = plr.viewpoint_x
      local cy = plr.viewpoint_y
      while cx == plr.viewpoint_x and cy == plr.viewpoint_y do
         sleep(300)
      end
   end

   _wait_for_move()
   sleep(3000) -- Give the player a chance to try this some more

   msg_box(tell_about_right_drag_move)

   _wait_for_move()
   sleep(3000) -- Give the player a chance to try this some more
   
   msg_box(congratulate_and_on_to_quarry)

   build_a_quarry()
end

function build_a_quarry()
   sleep(200)

   -- Teaching how to build a quarry and the nits and knacks of road building.
   msg_box(order_quarry_recap_how_to_build)

   -- Wait for the constructionsite to come up.
   -- TODO: this needs to be done better, but a wrapping of the constructionsite
   -- is needed for this. 
   -- TODO: check that the constructionsite is indeed for the correct building
   local done = false
   while not done do
      for idx,f in ipairs(first_quarry_field:region(6)) do
         if f.immovable and f.immovable.type == "constructionsite" then
            done = true
            break
         end
      end
      sleep(400)
   end

   msg_box(talk_about_roadbuilding)

   while #plr:get_buildings("quarry") < 1 do sleep(1400) end

   messages()
end

function messages()
   -- Teach the player about receiving messages
   sleep(10)

   send_message(teaching_about_messages)

   while #plr.inbox > 0 do sleep(200) end

   msg_box(closing_msg_window)

   -- TODO: the message window should be closed here first.
      
   -- Remove all stones
   local fields = first_quarry_field:region(6)
   while #fields > 0 do
      local idx = math.random(#fields)
      local f = fields[idx]
      local remove_field = true

      if f.immovable then
         temp, temp, n = f.immovable.name:find("stones(%d*)")
         if n then
            n = n + 0 -- trick to convert string to integer
            f.immovable:remove()
            if n > 1 then 
               remove_field = false
               wl.map.create_immovable(("stones%i"):format(n-1), f)
               sleep(150)
            end
         end
      end

      if remove_field then
         table.remove(fields, idx)
      end
   end

   -- Wait for message to arrive
   while #plr.inbox < 1 do sleep(300) end
   msg_box(conclude_messages)

   expansion()
end

function expansion() 
   -- Teach about expanding your territory.
   sleep(10)

   msg_box(introduce_expansion)

   while #conquer_field.owners < 1 do sleep(100) end

   conclusion()
end

function conclusion()
   -- Conclude the tutorial with final words and information
   -- on how to quit

   sleep(4000)
   msg_box(conclude_tutorial)

end

-- run(starting_infos)
run(build_lumberjack)

