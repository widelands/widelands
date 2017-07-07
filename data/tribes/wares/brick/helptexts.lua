function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Brick
      frisians = pgettext("frisians_ware", "Bricks are the best and most important building material. They are made out of clay and granite.")
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
