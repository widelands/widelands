function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Mead
      amazons = pgettext("amazons_ware", "Chocolate is produced by chocolate breweries."),
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
