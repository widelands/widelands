function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Milking Tongs
      atlanteans = pgettext("atlanteans_ware", "Milking tongs are used by the spider breeder to milk the spiders. They are produced by the toolsmith.")
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