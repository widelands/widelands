function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Spear
      amazons = pgettext("amazons_ware", "This spear has a stone spike. It is produced in a stone carvery and used in a warriors gathering – together with food – to train soldiers from attack level 0 to attack level 1.")
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
