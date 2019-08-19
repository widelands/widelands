function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Chisel
      default = pgettext("ware", "The chisel is an essential tool."),
      -- TRANSLATORS: Helptext for a ware: Chisel
      amazons = pgettext("amazons_ware", "Stonecarvers need a chisel. Make sure you’ve always got some in reserve! They are produced by the stonecarver itself."),

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
