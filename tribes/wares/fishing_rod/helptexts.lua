function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Fishing Rod
      default = pgettext("ware", "Fishing rods are needed by fishers to catch fish."),
      -- TRANSLATORS: Helptext for a ware: Fishing Rod
      barbarians = pgettext("barbarians_ware", "They are one of the basic tools produced in a metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill)."),
      -- TRANSLATORS: Helptext for a ware: Fishing Rod
      empire = pgettext("empire_ware", "They are produced by the toolsmith.")
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