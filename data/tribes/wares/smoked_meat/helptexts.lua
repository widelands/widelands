function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Smoked Meat
      atlanteans = pgettext("atlanteans_ware", "Smoked meat is made out of meat in a smokery. It is delivered to the mines and training sites (labyrinth and dungeon) where the miners and soldiers prepare a nutritious lunch for themselves."),
      -- TRANSLATORS: Helptext for a ware: Smoked Meat
      frisians = pgettext("frisians_ware", "Meat is smoked in a smokery. Smoked meat is then consumed by soldiers in training or turned into rations and meals for miners.")
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
