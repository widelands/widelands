function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Kitchen Tools
      barbarians = pgettext("barbarians_ware", "Kitchen tools are needed for preparing rations, snacks and meals. Be sure to have a metal workshop to produce this basic tool (but it ceases to be produced by the building if it is enhanced to an ax workshop and war mill)."),
      -- TRANSLATORS: Helptext for a ware: Kitchen Tools
      frisians = pgettext("frisians_ware", "Kitchen tools are needed for preparing rations and meals. The smoker also needs them."),
      -- TRANSLATORS: Helptext for a ware: Kitchen Tools
      empire = pgettext("empire_ware", "Kitchen tools are needed for preparing rations and meals. They are produced in a toolsmithy and used in taverns and inns.")
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
