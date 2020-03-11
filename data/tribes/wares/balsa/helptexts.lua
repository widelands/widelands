function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Balsa
      amazons =
         pgettext("amazons_ware", "Balsa wood is an important building material. Balsa trees are cut by an experienced woodcutter.")
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
