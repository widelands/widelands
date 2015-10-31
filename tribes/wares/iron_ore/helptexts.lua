function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Iron Ore
      default = pgettext("default_ware", "Iron ore is mined in iron mines. It is smelted in a smelting works to retrieve the iron.")
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