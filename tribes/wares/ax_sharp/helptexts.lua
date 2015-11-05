function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Sharp Ax
      barbarians = pgettext("barbarians_ware", "Young warriors are proud to learn to fight with this powerful weapon. It is heavier and sharper than the ordinary ax. It is produced in ax workshops and war mills. In training camps, it is used – together with food – to train soldiers from attack level 0 to level 1.")
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
