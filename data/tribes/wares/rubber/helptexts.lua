function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Rubber
      amazons = pgettext("amazons_ware", "Rubber is collected from special trees.")
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
