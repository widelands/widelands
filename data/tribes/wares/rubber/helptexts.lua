function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Rubber
      amazons = pgettext("amazons_ware", "Rubber is important for making warriors armor and shoes. Rubber trees are collected from experienced woodcutters.")
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
