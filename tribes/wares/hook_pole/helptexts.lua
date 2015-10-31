function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Hook Pole
      atlanteans = pgettext("atlanteans_ware", "This hook pole is used by the smoker to suspend all the meat and fish from the top of the smokery. It is created by the toolsmith.")
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