function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Old Fur Garment
      frisians = pgettext("frisians_ware", "Old garments can be turned into fur in a recycling center."),
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
