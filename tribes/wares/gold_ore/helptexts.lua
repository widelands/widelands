function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Gold Ore
      default = pgettext("ware", "Gold ore is mined in a goldmine. Smelted in a smelting works, it turns into gold which is used as a precious building material and to produce weapons and armor.")
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