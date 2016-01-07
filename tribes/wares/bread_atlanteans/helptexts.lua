function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Bread
      atlanteans = pgettext("atlanteans_ware", "This tasty bread is made in bakeries out of cornmeal, blackroot flour and water. It is appreciated as basic food by miners, scouts and soldiers in training sites (labyrinth and dungeon).")
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
