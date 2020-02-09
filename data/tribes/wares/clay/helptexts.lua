function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Clay
      frisians = pgettext("frisians_ware", "Clay is made out of water and mud to be turned into bricks, used in ship construction and to improve the charcoal kiln.")
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
