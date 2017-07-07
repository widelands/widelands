function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Scrap metal (iron)
      frisians = pgettext("frisians_ware", "Discarded weapon and armour can be recycled in a recycling centre to produce new tools, weapon and armour.")
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
