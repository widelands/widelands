function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Golden Helmet
      frisians = pgettext("frisians_ware", "A golden helmet protects warriors. It is produced in the armoursmithy and used to train soldiers from health level 1 to level 2.")
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
