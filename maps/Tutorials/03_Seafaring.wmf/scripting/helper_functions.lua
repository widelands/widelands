-- =================
-- Helper functions
-- =================

-- TODO: move common functions to tutorial_utils.lua or so
-- compare with other stuff (documentation, redundant functions etc.)


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
   if i.field then
      scroll_smoothly_to(i.field.trn.trn.trn.trn)
      -- TODO: improve this, depending on topleft/topright, scroll only left/right

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

   return o
end

-- A copy of prefilled_buildings from scripting/infrastructure.lua, but with
-- as much wares as possible, if not indicated otherwise
function filled_buildings(p, ...)
   for idx,bdescr in ipairs({...}) do
      b = p:place_building(bdescr[1], wl.Game().map:get_field(bdescr[2],bdescr[3]), false, true)
      -- Fill with workers
      if b.valid_workers then b:set_workers(b.valid_workers) end
      if bdescr.workers then b:set_workers(bdescr.workers) end
      -- Fill with soldiers
      if b.max_soldiers and b.set_soldiers then
         if bdescr.soldiers then
            b:set_soldiers(bdescr.soldiers)
         else
            b:set_soldiers({0,0,0,0}, b.max_soldiers)
         end
      elseif bdescr.soldiers then -- Must be a warehouse
         b:set_soldiers(bdescr.soldiers)
      end
      -- Fill with wares
      if bdescr.wares then b:set_wares(bdescr.wares)
      elseif b.valid_wares then b:set_wares(b.valid_wares) end
   end
end
