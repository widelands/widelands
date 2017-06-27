function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Double-edged Sword
      frisians = pgettext("frisians_ware", "The double-edged sword is the weapon used by level 3 warriors. Level 6 warriors are equipped with two of these ferocious swords.")
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
