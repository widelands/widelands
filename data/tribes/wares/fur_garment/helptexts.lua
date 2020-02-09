function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Fur Garment
      frisians = pgettext("frisians_ware", "Fur can be sewn into garments. They are used as basic armor. All new soldiers are clothed in a fur garment."),
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
