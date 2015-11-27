function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Bronze Ax
      barbarians = pgettext("barbarians_ware", "The bronze ax is considered a weapon that is hard to handle. Only skilled soldiers can use it. It is produced at the war mill and used in the training camp – together with food – to train soldiers from attack level 2 to level 3.")
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
