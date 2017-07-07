function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Fruit
      frisians = pgettext("frisians_ware", "Fruit are berries gathered from berry bushes by a fruit collector. They are used for rations and meals.")
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
