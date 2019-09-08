function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Fire Tongs
      default = pgettext("ware", "Fire tongs are the tools for smelting ores."),
      -- TRANSLATORS: Helptext for a ware: Fire Tongs
      atlanteans = pgettext("atlanteans_ware_fire_tongs", "They are used in the smelting works and produced by the toolsmith."),
      -- TRANSLATORS: Helptext for a ware: Fire Tongs
      frisians = pgettext("frisians_ware", "They are used in the furnace and the brick kiln and produced by the blacksmithy."),
      -- TRANSLATORS: Helptext for a ware: Fire Tongs
      barbarians = pgettext("barbarians_ware", "They are used in the smelting works and produced by the metal workshop (but they cease to be produced by the building if it is enhanced to an ax workshop and war mill)."),
      -- TRANSLATORS: Helptext for a ware: Fire Tongs
      empire = pgettext("empire_ware_fire_tongs", "They are used in the smelting works and produced by the toolsmith.")
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
