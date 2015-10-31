function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Blackroot
      atlanteans = pgettext("atlanteans_ware", "Blackroots are a special kind of root produced at blackroot farms and processed in mills. The Atlanteans like their strong taste and use their flour for making bread.")
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