function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Fur Garment
      amazons = pgettext("amazons_ware", "Rubber can be bound into a tunic. They are used as basic armor. All new soldiers are clothed in a tunic."),
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
