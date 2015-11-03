function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Gilded Armor
      empire = pgettext("empire_ware", "The gilded armor is the strongest armor an Empire soldier can have. It is produced in the armor smithy and used in the training camp – together with food – to train soldiers from health level 3 to level 4.")
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
