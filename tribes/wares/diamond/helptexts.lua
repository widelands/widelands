function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Diamond
      atlanteans = pgettext("atlanteans_ware", "These wonderful diamonds are used to build some exclusive buildings. They are mined in a crystal mine.")
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
