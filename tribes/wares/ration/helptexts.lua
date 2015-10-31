function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Ration
      default = pgettext("ware", "A small bite to keep miners strong and working. The scout also consumes rations on his scouting trips."),
      -- TRANSLATORS: Helptext for a ware: Ration
      barbarians = pgettext("barbarians_ware", "Rations are produced in a tavern, an inn or a big inn out of fish or meat or pitta bread."),
      -- TRANSLATORS: Helptext for a ware: Ration
      empire = pgettext("empire_ware", "Rations are produced in a tavern out of fish or meat or bread.")
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