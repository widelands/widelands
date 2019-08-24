function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Padded Protector
      amazons = pgettext("amazons_ware", "A padded protector is a strong and durable armor. it is produced by the dressmaker and used to train soldiers defence from level 1 to level 2.")
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
