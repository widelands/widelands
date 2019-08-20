function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Blackroot
      amazons = pgettext("amazons_ware", "Cassavaroots are a special kind of root produced at cassava farms. The Amazons like their strong taste for making bread and they feed also tapirs with it.")
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
