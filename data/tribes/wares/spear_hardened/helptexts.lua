function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Advanced Spear
      amazons = pgettext("amazons_ware", "This is an hardened spear with a stone tip. It is produced in a stone carvery and used in a warriors gathering – together with food – to train soldiers from attack level 1 to level 2.")
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
