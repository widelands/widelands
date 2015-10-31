function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Wine
      empire = pgettext("empire_ware", "This tasty wine is drunk by the miners working the marble and gold mines. It is produced in a winery.")
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