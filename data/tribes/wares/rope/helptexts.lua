function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Rope
      amazons = pgettext("amazons_ware", "This rope is made out of liana. It is used for armor dresses and to construct buildings.")
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
