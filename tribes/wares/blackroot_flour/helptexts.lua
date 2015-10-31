function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Blackroot Flour
      atlanteans = pgettext("atlanteans_ware", "Blackroot Flour is produced in mills out of blackroots. It is used in bakeries to make a tasty bread.")
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