function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Double Trident
      atlanteans = pgettext("atlanteans_ware", "The double trident is one of the best tridents produced by the Atlantean weapon smithy. It is used in a dungeon – together with food – to train soldiers from attack level 2 to level 3.")
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