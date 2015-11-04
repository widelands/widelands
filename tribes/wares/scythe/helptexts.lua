function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Scythe
      default = pgettext("ware", "The scythe is the tool of the farmers."),
      -- TRANSLATORS: Helptext for a ware: Scythe
      atlanteans = pgettext("atlanteans_ware", "Scythes are produced by the toolsmith."),
      -- TRANSLATORS: Helptext for a ware: Scythe
      barbarians = pgettext("barbarians_ware", "Scythes are produced by the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill)."),
      -- TRANSLATORS: Helptext for a ware: Scythe
      empire = pgettext("empire_ware", "Scythes are produced by the toolsmith.")
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
