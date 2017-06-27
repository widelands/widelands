function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Long Sword
      frisians = pgettext("frisians_ware", "The long sword is the weapon used by level 1 warriors. Level 4 warriors are equipped with two long swords.")
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
