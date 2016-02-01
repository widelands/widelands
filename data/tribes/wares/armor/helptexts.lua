function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Armor
      empire = pgettext("empire_ware", "Basic armor for Empire soldiers. It is produced in the armor smithy and used in the training camp – together with food – to train soldiers from health level 1 to level 2.")
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
