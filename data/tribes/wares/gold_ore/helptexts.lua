function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Gold Ore
      default = pgettext("ware", "Gold ore is mined in a gold mine."),
      -- TRANSLATORS: Helptext for a ware: Gold Ore
      atlanteans = pgettext("atlanteans_ware", "Smelted in a smelting works, it turns into gold which is used as a precious building material and to produce weapons and armor."),
      -- TRANSLATORS: Helptext for a ware: Gold Ore
      barbarians = pgettext("barbarians_ware", "Smelted in a smelting works, it turns into gold which is used as a precious building material and to produce weapons and armor."),
      -- TRANSLATORS: Helptext for a ware: Gold Ore
      frisians = pgettext("frisians_ware", "Smelted in a furnace, it turns into gold which is used as a precious building material and to produce weapons and armor."),
      -- TRANSLATORS: Helptext for a ware: Gold Ore
      empire = pgettext("empire_ware", "Smelted in a smelting works, it turns into gold which is used as a precious building material and to produce weapons and armor.")
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
