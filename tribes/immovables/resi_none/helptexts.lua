-- TODO(GunChleoc): This is unused

function immovable_helptext(tribe)
   local helptext = {
      -- #TRANSLATORS: Helptext for a resource: No resources
      default = "There are no resources in the ground here."
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