function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: machete
      default = pgettext("ware", "The machete is the tool of the liana cutters."),
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
