function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Tabard
      amazons = pgettext("amazons_ware", "A padded vest is a light but durable armor. it is produced by the dressmaker and used to train soldiers defence from level 0 to level 1.")
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
