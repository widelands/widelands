function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Honey Bread
      frisians = pgettext("frisians_ware", "This bread is sweetened with honey. It is consumed by the most experienced miners and in advanced soldier training.")
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
