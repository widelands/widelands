function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Steel Shield
      atlanteans = pgettext("atlanteans_ware", "This steel shield is produced in the armor smithy and used in the labyrinth – together with food – to train soldiers from defense level 0 to level 1.")
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
