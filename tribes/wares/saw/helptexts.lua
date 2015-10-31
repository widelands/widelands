function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Saw
      atlanteans = pgettext("atlanteans_ware", "The saw is needed by the woodcutter and the toolsmith. It is produced by the toolsmith."),
      -- TRANSLATORS: Helptext for a ware: Saw
      empire = pgettext("empire_ware", "This saw is needed by the sawyer. It is produced by the toolsmith.")
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