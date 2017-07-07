function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Fur Clothes
      frisians = pgettext("frisians_ware", "Fur can be sewed into clothes. They are used as basic armour. All new soldiers are clothed in furs."),
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
