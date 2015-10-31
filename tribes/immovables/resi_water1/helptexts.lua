-- TODO(GunChleoc): This is unused

function immovable_helptext(tribe)
   local helptext = {
      -- #TRANSLATORS: Helptext for a resource: Water
      default = "There is water in the ground here that can be pulled up by a well."
   }
   local result = ""
   if tribe then
      result = helptext[tribe]
   else
      result = helptext["default"]
   end
   if (result == nil) then result = "" end
   return result
end