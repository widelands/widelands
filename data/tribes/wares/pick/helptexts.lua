function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Pick
      atlanteans = pgettext("ware", "Picks are used by stonecutters and miners. They are produced by the toolsmith."),
      -- TRANSLATORS: Helptext for a ware: Pick
      barbarians = pgettext("barbarians_ware", "Picks are used by stonemasons and miners. They are produced in the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill)."),
      -- TRANSLATORS: Helptext for a ware: Pick
      empire = pgettext("empire_ware", "Picks are used by stonemasons and miners. They are produced by the toolsmith."),
      frisians = pgettext("frisians_ware", "Picks are used by stonemasons and miners.")
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
