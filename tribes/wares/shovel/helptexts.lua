function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Shovel
      default = pgettext("ware", "Shovels are needed for the proper handling of plants."),
      -- TRANSLATORS: Helptext for a ware: Shovel
      atlanteans = pgettext("atlanteans_ware", "Therefore the forester and the blackroot farmer use them. They are produced by the toolsmith."),
      -- TRANSLATORS: Helptext for a ware: Shovel
      barbarians = pgettext("barbarians_ware", "Therefore the gardener and the ranger use them. Produced at the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill)."),
      -- TRANSLATORS: Helptext for a ware: Shovel
      empire = pgettext("empire_ware", "Therefore the forester and the vine farmer use them. They are produced by the toolsmith.")
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