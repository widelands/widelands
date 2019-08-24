function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Swift boots
      amazons = pgettext("amazons_ware", "Swift boots are a light and durable footwear for soldiers to be quicker on the feet. They are produced by the dressmaker and used to train soldiers evade from level 1 to level 2.")
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
