function ware_helptext(tribe)
   local helptext = {
      default = pgettext("ware", "Custom ware does nothing"),
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
