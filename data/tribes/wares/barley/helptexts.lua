function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Barley
      frisians = pgettext("frisians_ware", "Barley is a slow-growing grain that is used for baking bread and brewing beer. It is also eaten by reindeer.")
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
