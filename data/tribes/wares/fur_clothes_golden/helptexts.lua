function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Golden Fur Clothes
      frisians = pgettext("frisians_ware", "Ordinary fur clothes can be decorated with iron and gold. Such clothes are the best armour."),
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
