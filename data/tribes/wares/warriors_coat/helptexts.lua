function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Fur Garment
      amazons = pgettext("amazons_ware", "Ordinary tunics can be decorated and reinforced with gold and wood chunks. Such clothes are the best armor."),
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
