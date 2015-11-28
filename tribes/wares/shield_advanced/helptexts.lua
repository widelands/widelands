function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Advanced Shield
      atlanteans = pgettext("atlanteans_ware", "These advanced shields are used by the best soldiers of the Atlanteans. They are produced in the armor smithy and used in the labyrinth – together with food – to train soldiers from defense level 1 to level 2.")
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
