function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Golden Tabard
      atlanteans = pgettext("atlanteans_ware", "Golden tabards are produced in Atlantean weaving mills out of gold thread. They are used in the labyrinth – together with food – to train soldiers from health level 0 to level 1.")
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