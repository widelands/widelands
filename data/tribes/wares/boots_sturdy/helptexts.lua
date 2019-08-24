function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Sturdy boots
      amazons = pgettext("amazons_ware", "Sturdy boots are an excellent footwear for soldiers to be quicker on the feet. They are produced by the dressmaker and used to train soldiers evade from level 0 to level 1.")
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
