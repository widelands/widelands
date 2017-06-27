function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Smoked Fish
      atlanteans = pgettext("atlanteans_ware", "As no Atlantean likes raw fish, smoking it in a smokery is the most common way to make it edible."),
      frisians = pgettext("frisians_ware", "Fish is smoked in a tannery. Smoked fish is then consumed by warriors in training or turned into rations and meals for miners.")
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
