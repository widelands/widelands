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
   wl.Game().desired_speed = 1000

   local blocker = UserInputDisabler:new()

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

   wl.ui.set_user_input_allowed(true)
   plr:message_box(i.title, i.body, i)

   blocker:lift_blocks()

   local o = _try_add_objective(i)

   sleep(130)

   return o
end

function send_message(i)
   plr:send_message(i.title, i.body, i)

   local o = _try_add_objective(i)
   sleep(130)
   return o
end

-- Remove all rocks in a given environment. This is done
-- in a loop for a nice optical effect
function remove_all_rocks(fields, g_sleeptime)
   local sleeptime = g_sleeptime or 150
   while #fields > 0 do
      local idx = math.random(#fields)
      local f = fields[idx]
      local remove_field = true

      if f.immovable then
         local n = f.immovable.descr.name:match("greenland_stones(%d*)")
         if n then
            n = tonumber(n)
            f.immovable:remove()
            if n > 1 then
               remove_field = false
               map:place_immovable("greenland_stones" .. n-1, f)
            end
            sleep(sleeptime)
         end
      end

      if remove_field then
         table.remove(fields, idx)
      end
   end
end


