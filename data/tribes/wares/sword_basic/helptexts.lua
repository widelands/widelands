function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Basic sword
      frisians = pgettext("frisians_ware", "The long sword is the weapon used by level 0 warriors. All new warriors are equipped with it in a barracks.")
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
