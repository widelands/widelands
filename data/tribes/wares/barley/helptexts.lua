function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Barley
      frisians = pgettext("frisians_ware", "Barley is a slow-growing grain that is used to bake bread and brew beer. It is also eaten by reindeer.")
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
