function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Scrap Iron
      frisians = pgettext("frisians_ware", "Discarded weapons and armor can be recycled in a recycling center to produce new tools, weapon and armor.")
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
