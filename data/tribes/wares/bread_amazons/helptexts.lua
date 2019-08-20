function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Bread
      amazons = pgettext("amazons_ware", "This tasty bread is made in cookeries out of cassava root and water. It is appreciated as basic food by miners, scouts and soldiers in training sites (labyrinth and dungeon).")
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
