function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Coal
      default = pgettext("ware", "Coal is mined in coal mines or produced out of logs by a charcoal kiln."),
      -- TRANSLATORS: Helptext for a ware: Coal
      atlanteans = pgettext("atlanteans_ware", "The Atlantean fires in smelting works, armor smithies and weapon smithies are fed with coal."),
      -- TRANSLATORS: Helptext for a ware: Coal
      barbarians = pgettext("barbarians_ware", "The fires of the Barbarians are usually fed with coal. Consumers are several buildings: lime kiln, smelting works, ax workshop, war mill, and helm smithy."),
      -- TRANSLATORS: Helptext for a ware: Coal
      empire = pgettext("empire_ware", "The fires of the Empire smelting works, armor smithies and weapon smithies are usually fed with coal.")
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
