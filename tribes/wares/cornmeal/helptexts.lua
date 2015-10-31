function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Cornmeal
      atlanteans = pgettext("atlanteans_ware", "Cornmeal is produced in a mill out of corn and is one of three parts of the Atlantean bread produced in bakeries.")
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