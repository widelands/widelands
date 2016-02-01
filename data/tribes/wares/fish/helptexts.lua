function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Fish
      atlanteans = pgettext("atlanteans_ware", "Fish is one of the biggest food resources of the Atlanteans. It has to be smoked in a smokery before being delivered to mines, training sites and scouts."),
      -- TRANSLATORS: Helptext for a ware: Fish
      barbarians = pgettext("barbarians_ware", "Besides pitta bread and meat, fish is also a foodstuff for the Barbarians. It is used in the taverns, inns and big inns and at the training sites (training camp and battle arena)."),
      -- TRANSLATORS: Helptext for a ware: Fish
      empire = pgettext("empire_ware", "Besides bread and meat, fish is also a foodstuff for the Empire. Fish are used in taverns, inns and training sites (arena, colosseum, training camp).")
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
