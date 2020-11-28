function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Wheat
      default = pgettext("ware", "Rye is essential for producing europeans bread."),
      -- TRANSLATORS: Helptext for a ware: Wheat
      europeans = pgettext("europeans_ware", "Rye is produced by farms and consumed by mills."),
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
