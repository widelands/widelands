function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Iron Ore
      default = pgettext("default_ware", "Iron ore is mined in iron mines."),
      -- TRANSLATORS: Helptext for a ware: Iron Ore
      atlanteans = pgettext("atlanteans_ware", "It is smelted in a smelting works to retrieve the iron."),
      -- TRANSLATORS: Helptext for a ware: Iron Ore
      barbarians = pgettext("barbarians_ware", "It is smelted in a smelting works to retrieve the iron."),
      -- TRANSLATORS: Helptext for a ware: Iron Ore
      frisians = pgettext("frisians_ware", "It is smelted in a furnace to retrieve the iron."),
      -- TRANSLATORS: Helptext for a ware: Iron Ore
      empire = pgettext("empire_ware", "It is smelted in a smelting works to retrieve the iron.")

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
