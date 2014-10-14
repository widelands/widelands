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
