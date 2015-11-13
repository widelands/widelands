function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Broad Ax
      barbarians = pgettext("barbarians_ware", "The broad ax is produced by the ax workshop and the war mill. It is used in the training camp – together with food – to train soldiers with a fundamental fighting knowledge from attack level 1 to attack level 2.")
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
