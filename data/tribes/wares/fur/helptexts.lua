function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Fur
      frisians = pgettext("frisians_ware", "Fur is won from reindeer in a reindeer farm. It is woven into cloth or turned into fur garments for soldiers."),
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
