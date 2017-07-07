function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Meat
      default = pgettext("ware", "Meat contains a lot of energy, and it is obtained from wild game taken by hunters."),
      -- TRANSLATORS: Helptext for a ware: Meat
      atlanteans = pgettext("atlanteans_ware", "Meat has to be smoked in a smokery before being delivered to mines and training sites (dungeon and labyrinth)."),
      -- TRANSLATORS: Helptext for a ware: Meat
      frisians = pgettext("frisians_ware", "Meat has to be smoked in a tannery before being delivered to taverns, drinking halls and training sites."),
      -- TRANSLATORS: Helptext for a ware: Meat
      barbarians = pgettext("barbarians_ware", "Meat is used in the taverns, inns and big inns to prepare rations, snacks and meals for the miners. It is also consumed at the training sites (training camp and battle arena)."),
      -- TRANSLATORS: Helptext for a ware: Meat
      empire = pgettext("empire_ware", "Meat can also be obtained as pork from piggeries. It is used in the inns and taverns to prepare lunch for the miners and is consumed at the training sites (arena, colosseum, training camp).")
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
