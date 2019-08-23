function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Helmet
      amazons = pgettext ("amazons_ware", "A helmet is a basic equipment to protect soldiers. It is produced by the dressmaker and used to train soldiers from health level 1 to level 2.")
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
