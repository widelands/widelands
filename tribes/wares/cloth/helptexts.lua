function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Cloth
      barbarians = pgettext("barbarians_ware", "Cloth is needed for Barbarian ships. It is produced out of thatch reed."),
      -- TRANSLATORS: Helptext for a ware: Cloth
      empire = pgettext("empire_ware", "Cloth is needed to build several buildings. It is also consumed in the armor smithy.")
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