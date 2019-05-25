function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Iron
      default = pgettext("ware", "Iron is smelted out of iron ores."),
      -- TRANSLATORS: Helptext for a ware: Iron
      atlanteans = pgettext("atlanteans_ware", "It is produced by the smelting works and used in the toolsmithy, armor smithy and weapon smithy."),
      -- TRANSLATORS: Helptext for a ware: Iron
      barbarians = pgettext("barbarians_ware", "It is produced by the smelting works and used to produce weapons and tools in the metal workshop, ax workshop, war mill and helm smithy."),
      -- TRANSLATORS: Helptext for a ware: Iron
      empire = pgettext("empire_ware", "It is produced by the smelting works. Weapons, armor and tools are made of iron."),
      -- TRANSLATORS: Helptext for a ware: Iron
      frisians = pgettext("frisians_ware", "It is produced by the furnace. Tools and weapons are made of iron. It is also used as jewellery for fur garment armor to give it a silver shine.")
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
