function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Beer
      barbarians = pgettext("barbarians_ware", "Beer is produced in micro breweries and used in inns and big inns to produce snacks."),
      -- TRANSLATORS: Helptext for a ware: Beer
      empire = pgettext("empire_ware", "This beer is produced in a brewery out of wheat and water. It is consumed by miners in coal and iron mines.")
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
