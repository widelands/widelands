function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Mead
      frisians = pgettext("frisians_ware", "Mead is produced by mead breweries. Soldiers drink mead during advanced training."),
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
