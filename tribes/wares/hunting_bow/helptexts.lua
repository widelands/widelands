function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Hunting Bow
      atlanteans = pgettext("atlanteans_ware", "This bow is used by the Atlantean hunter. It is produced by the toolsmith.")
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