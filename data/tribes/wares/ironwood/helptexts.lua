function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Ironwood
      amazons = pgettext("amazons_ware", "This very hard wood is as hard as iron and it is used for several buildings. It is cut by a very experienced woodcutter.")
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
