function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Long Trident
      atlanteans = pgettext("atlanteans_ware", "The long trident is the first trident in the training of soldiers. It is produced in the weapon smithy and used in the dungeon – together with food – to train soldiers from attack level 0 to level 1.")
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
