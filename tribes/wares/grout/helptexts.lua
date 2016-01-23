function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Grout
      barbarians = pgettext("barbarians_ware", "Granite can be processed into grout which provides a solid, non burning building material. Grout is produced in a lime kiln.")
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
