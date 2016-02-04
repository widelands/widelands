function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Blackwood
      barbarians = pgettext("barbarians_ware", "This fire-hardened wood is as hard as iron and it is used for several buildings. It is produced out of logs in the wood hardener.")
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
