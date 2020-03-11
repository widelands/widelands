function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Needles
      frisians = pgettext("frisians_ware", "Needles are used by seamstresses to sew cloth and fur garments."),
      -- TRANSLATORS: Helptext for a ware: Needles
      amazons = pgettext("amazons_ware", "Needles are used by dressmakers to make dresses, boots and armor for the soldiers."),
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
