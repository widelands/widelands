function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Gold Thread
      atlanteans = pgettext("atlanteans_ware", "This thread, made of gold by the gold spinning mill, is used for weaving the exclusive golden tabard in the weaving mill.")
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