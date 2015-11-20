function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Fishing Net
      atlanteans = pgettext("atlanteans_ware", "The fishing net is used by the fisher and produced by the toolsmith.")
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
