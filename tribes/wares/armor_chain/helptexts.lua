function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Chain Armor
      empire = pgettext("empire_ware", "The chain armor is a medium armor for Empire soldiers. It is produced in an armor smithy and used in a training camp – together with food – to train soldiers from health level 2 to level 3.")
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