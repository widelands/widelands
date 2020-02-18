function ware_helptext(tribe)
   local helptext = {
      barbarians = pgettext("custom_ware", "This is a repaced ware.")
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
