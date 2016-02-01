function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Grape
      empire = pgettext("empire_ware", "These grapes are the base for a tasty wine. They are harvested in a vineyard and processed in a winery.")
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
