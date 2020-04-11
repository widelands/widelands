-- =================
-- Helper functions
-- =================

-- Check if player seen any field of given array of fields
function any_field_seen(player, fields)
   for k,v in pairs(fields) do
      if player:seen_field(v) then
         return true
      end
   end
   return false
end
