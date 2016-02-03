function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Warhelm
      barbarians = pgettext("barbarians_ware", "A warhelm is an enhanced armor for Barbarian soldiers. It is produced in a helm smithy and used in a training camp – together with food – to train soldiers from health level 1 to level 2.")
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
