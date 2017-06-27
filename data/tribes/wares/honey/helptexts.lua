function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Grape
      frisians = pgettext("frisians_ware", "Honey is produced by bees belonging to a bee-keeper. It is used to bake sweet bread and brew mead.")
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
