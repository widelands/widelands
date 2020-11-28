function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Iron Ore
      default = pgettext("default_ware", "Ore is mined in ore mines. It is smelted in a smelting works to retrieve the iron and gold.")
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
