function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Chain Armor
      amazons = pgettext("amazons_ware", "The light wooden armor is a basic armor for amazons soldiers. It is produced by a dressmaker and used in a training glade – together with food – to train soldiers from health level 0 to level 1.")
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
