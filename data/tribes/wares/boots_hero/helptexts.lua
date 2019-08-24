function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Hero boots
      amazons = pgettext("amazons_ware", "Hero boots! The only suitable footwear for a real hero. They are produced by the dressmaker and used to train soldiers evade from level 2 to level 3.")
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
