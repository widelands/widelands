function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Meal
      barbarians = pgettext("barbarians_ware", "A meal is made out of pitta bread, strong beer and fish/meat in a big inn. This substantial food is exactly what workers in a deeper mine need."),
      -- TRANSLATORS: Helptext for a ware: Meal
      empire = pgettext("empire_ware", "A real meal is made in inns out of bread and fish/meat. It satisfies the needs of miners in deep mines.")
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
