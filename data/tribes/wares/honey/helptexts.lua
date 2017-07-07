function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Honey
      frisians = pgettext("frisians_ware", "Honey is produced by bees belonging to a bee-keeper. It is used to bake honey bread and brew mead.")
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
