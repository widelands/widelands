-- =================
-- Helper functions
-- =================

-- A copy of prefilled_buildings from scripting/infrastructure.lua, but with
-- as much wares as possible, if not indicated otherwise, and only one soldier
function filled_buildings_one_soldier(p, ...)
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
            -- TODO(wl-zocker): this creates "1 soldier (+4)", but I want a capacity of 1 (i.e. "1 soldier"). See https://bugs.launchpad.net/widelands/+bug/1387310
            b:set_soldiers({0,0,0,0}, 1)
         end
      elseif bdescr.soldiers then -- Must be a warehouse
         b:set_soldiers(bdescr.soldiers)
      end
      -- Fill with wares
      if bdescr.wares then b:set_wares(bdescr.wares)
      elseif b.valid_wares then b:set_wares(b.valid_wares) end
      if bdescr.inputs then b:set_inputs(bdescr.inputs)
      elseif b.valid_inputs then b:set_inputs(b.valid_inputs) end
   end
end
