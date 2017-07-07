function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Basic sword
      frisians = pgettext("frisians_ware", "The basic sword is the weapon used by level 0 soldiers. All new soldiers are equipped with it in a barracks.")
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
