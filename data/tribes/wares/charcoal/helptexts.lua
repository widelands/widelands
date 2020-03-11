function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Coal
      amazons = pgettext("amazons_ware", "Charcoal produced out of logs by a charcoal burner. The Amazons use charcoal for drying their food and smelting gold."),
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
