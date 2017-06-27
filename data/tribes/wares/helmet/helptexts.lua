function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Helmet
      barbarians = pgettext("barbarians_ware", "A helmet is a basic tool to protect soldiers. It is produced in the helm smithy and used in the training camp – together with food – to train soldiers from health level 0 to level 1."),
      -- TRANSLATORS: Helptext for a ware: Helmet
      frisians = pgettext("frisians_ware", "A helmet is a basic tool to protect warriors. It is produced in the armour smithy and used to train warriors from health level 0 to level 1.")
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
