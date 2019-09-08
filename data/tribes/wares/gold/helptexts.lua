function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Gold
      default = pgettext("ware", "Gold is the most valuable of all metals, and it is smelted out of gold ore."),
      -- TRANSLATORS: Helptext for a ware: Gold
      atlanteans = pgettext("atlanteans_ware", "It is produced by the smelting works and used by the armor smithy, the weapon smithy and the gold spinning mill."),
      -- TRANSLATORS: Helptext for a ware: Gold
      barbarians = pgettext("barbarians_ware", "Only very important things are embellished with gold. It is produced by the smelting works and used as a precious building material and to produce different axes (in the war mill) and different parts of armor (in the helm smithy)."),
      -- TRANSLATORS: Helptext for a ware: Gold
      frisians = pgettext("frisians_ware", "Only very important things are embellished with gold. It is produced by the furnace and is used to produce better swords and the best helmets. The best armor is also decorated with gold."),
      -- TRANSLATORS: Helptext for a ware: Gold
      empire = pgettext("empire_ware", "It is produced by the smelting works. Armor and weapons are embellished with gold in the armor smithy and the weapon smithy.")
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
