function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Steel Trident
      atlanteans = pgettext("atlanteans_ware", "This is the medium trident. It is produced in the weapon smithy and used by advanced soldiers in the dungeon – together with food – to train from attack level 1 to level 2.")
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
