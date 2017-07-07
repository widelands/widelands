function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Curved Sword
      frisians = pgettext("frisians_ware", "The curved sword is the weapon used by level 2 soldier. Level 5 soldiers are equipped with two curved swords.")
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
