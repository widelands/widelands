function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Fur
      frisians = pgettext("frisians_ware", "Fur is won from reindeer in a reindeer farm. It is spun into cloth or turned into fur clothes for soldiers."),
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
