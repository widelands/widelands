function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Tabard
      atlanteans = pgettext("atlanteans_ware", "A tabard and a light trident are the basic equipment for young soldiers. Tabards are produced in the weaving mill.")
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