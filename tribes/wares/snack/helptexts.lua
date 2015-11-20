function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Snack
      barbarians = pgettext("barbarians_ware", "A bigger morsel than the ration to provide miners in deep mines. It is produced in an inn or a big inn out of fish/meat, pitta bread and beer.")
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
