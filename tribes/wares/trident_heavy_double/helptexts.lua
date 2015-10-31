function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Heavy Double Trident
      atlanteans = pgettext("atlanteans_ware", "This is the most dangerous weapon of the Atlantean military. Only the best of the best soldiers may use it. It is produced in the weapon smithy and used in the dungeon – together with food – to train soldiers from attack level 3 to level 4.")
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