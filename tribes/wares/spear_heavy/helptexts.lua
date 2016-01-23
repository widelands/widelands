function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Heavy Spear
      empire = pgettext("empire_ware", "This is a strong spear with a steel-tip and a little blade. It is produced in the weapon smithy and used in the training camp – together with food – train soldiers from attack level 2 to level 3.")
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
