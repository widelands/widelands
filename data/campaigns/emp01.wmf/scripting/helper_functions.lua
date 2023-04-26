-- Helper functions

-- Remove any tree which blocks needed build space for
-- a given table 'fields' containing map coordinates
function remove_trees(fields)
   for k, v in pairs(fields) do
      local f = wl.Game().map:get_field(v[1],v[2])
      if f.immovable and f.immovable:has_attribute("tree") then
         f.immovable:remove()
      end
   end
end
