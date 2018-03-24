function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Basket
      empire = pgettext("empire_ware", "This basket is needed by the vine farmer for harvesting the grapes. It is produced by the toolsmith."),
      frisians = pgettext("frisians_ware", "Baskets are needed by the fruit collector to gather berries. They are woven from reed and wood by the blacksmith."),
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
