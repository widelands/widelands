function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Golden Fur Garment
      frisians = pgettext("frisians_ware", "Ordinary fur garments can be decorated with iron and gold. Such clothes are the best armor."),
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
