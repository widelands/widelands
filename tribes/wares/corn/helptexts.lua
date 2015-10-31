function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Corn
      atlanteans = pgettext("atlanteans_ware", "This corn is processed in the mill into fine cornmeal that every Atlantean baker needs for a good bread. Also horse and spider farms need to be provided with corn.")
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