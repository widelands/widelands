function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: War Spear
      empire = pgettext("empire_ware", "This is the best and sharpest weapon the Empire weaponsmith creates for the soldiers. It is used in the training camp – together with food – to train soldiers from attack level 3 to level 4.")
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
