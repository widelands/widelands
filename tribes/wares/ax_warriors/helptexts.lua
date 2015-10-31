function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Warrior’s Ax
      barbarians = pgettext("barbarians_ware", "The warrior’s ax is the most dangerous of all Barbarian weapons. Only a few warriors ever were able to handle those huge and powerful axes. It is produced in a war mill and used – together with food – in a training camp to train soldiers from attack level 4 to level 5.")
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