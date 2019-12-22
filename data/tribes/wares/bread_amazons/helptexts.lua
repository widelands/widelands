function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Bread
      amazons = pgettext("amazons_ware", "This tasty bread is made in cookeries out of cassava root and water. It is the basic ingredient of a ration that is made by a food preserver. Soldiers like it too.")
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
