function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Wool
      empire = pgettext("empire_ware", "Wool is the hair of sheep. Weaving mills use it to make cloth.")
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