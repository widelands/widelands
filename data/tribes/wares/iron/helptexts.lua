function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Iron
      default = pgettext("ware", "Iron is extracted from iron ore in the smelting works."),
      -- TRANSLATORS: Helptext for a ware: Iron
      atlanteans = pgettext("atlanteans_ware", "It is used in the toolsmithy, armor smithy and weapon smithy."),
      -- TRANSLATORS: Helptext for a ware: Iron
      barbarians = pgettext("barbarians_ware", "It is used to produce weapons and tools in the metal workshop, ax workshop, war mill and helm smithy."),
      -- TRANSLATORS: Helptext for a ware: Iron
      empire = pgettext("empire_ware", "Weapons, armor and tools are made of iron."),
      frisians = pgettext("frisians_ware", "Tools and weapons are made of iron. It is also used as jewellery for fur cloth armour to give it a silver shine.")
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
