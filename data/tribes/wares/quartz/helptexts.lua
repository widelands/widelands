function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Quartz
      atlanteans = pgettext("atlanteans_ware", "These transparent quartz gems are used to build some exclusive buildings. They are produced in a crystal mine.")
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
