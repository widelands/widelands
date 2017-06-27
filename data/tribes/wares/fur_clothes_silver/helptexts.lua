function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Silver Fur Clothes
      frisians = pgettext("frisians_ware", "Ordinary fur clothes can be decorated with iron to give them a silvery shine. These clothes make good armour."),
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
