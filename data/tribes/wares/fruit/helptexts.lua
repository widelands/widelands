function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Grape
      frisians = pgettext("frisians_ware", "Fruit are berries and mushrooms gathered from berry bushes and the mushrooms growing under trees.")
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
