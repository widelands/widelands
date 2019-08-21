function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Advanced Shield
      amazons = pgettext("amazons_ware", "Stonebowls are used to wash gold and to smelt the gold dust.")
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
