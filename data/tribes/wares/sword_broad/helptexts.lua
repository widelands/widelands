function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Broadsword
      frisians = pgettext("frisians_ware", "The broadsword is the weapon used by level 2 soldiers. Level 5 soldiers are equipped with a broadsword and a double-edged sword.")
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
