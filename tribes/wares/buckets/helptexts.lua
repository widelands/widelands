function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Buckets
      atlanteans = pgettext("atlanteans_ware", "Big buckets for the fish breeder – produced by the toolsmith.")
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