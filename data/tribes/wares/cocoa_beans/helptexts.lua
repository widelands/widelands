function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Cocoa beans
      amazons = pgettext("amazons_ware", "Cocoa beans are gathered from cocoa bushes by a cocoa farmer. They are used for producing chocolate.")
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
