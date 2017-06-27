function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Grout
      barbarians = pgettext("frisians_ware", "Clay is made out of water and mud to be turned into bricks.")
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
