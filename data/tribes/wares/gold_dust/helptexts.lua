function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Gold Dust
      default = pgettext("ware", "Gold dust is washed in a gold digger dwelling."),
      -- TRANSLATORS: Helptext for a ware: Gold Dust
      amazons = pgettext("amazons_ware", "Smelted in a furnace, it turns into gold which is used as a precious building material and to produce armor."),
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
