function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Granite
      default = pgettext("ware", "Granite is a basic building material."),
      -- TRANSLATORS: Helptext for a ware: Granite
      atlanteans = pgettext("atlanteans_ware", "The Atlanteans produce granite blocks in quarries and crystal mines."),
      -- TRANSLATORS: Helptext for a ware: Granite
      barbarians = pgettext("barbarians_ware", "The Barbarians produce granite blocks in quarries and granite mines."),
      -- TRANSLATORS: Helptext for a ware: Granite
      empire = pgettext("empire_ware", "The Empire produces granite blocks in quarries and marble mines.")
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
