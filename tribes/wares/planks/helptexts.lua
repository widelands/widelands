function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Planks
      default = pgettext("ware", "Planks are an important building material."),
      -- TRANSLATORS: Helptext for a ware: Planks
      atlanteans =
			pgettext("atlanteans_ware_planks", "They are produced out of logs by the sawmill.") .. " " ..
			pgettext("atlanteans_ware", "The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships."),
      -- TRANSLATORS: Helptext for a ware: Planks
      empire =
			pgettext("empire_ware_planks", "They are produced out of logs by the sawmill.") .. " " ..
			pgettext("empire_ware", "They are also used by the weapon smithy.")
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
