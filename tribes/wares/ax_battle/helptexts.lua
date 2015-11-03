function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Battle Ax
      barbarians = pgettext("barbarians_ware", "This is a dangerous weapon the Barbarians are able to produce. It is produced in the ax factory. Only trained warriors are able to wield such a weapon. It is used – together with food – in the training camp to train soldiers from attack level 3 to 4.")
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
