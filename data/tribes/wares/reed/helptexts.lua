function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Reed
      barbarians = pgettext("barbarians_ware", "Reed is produced in a reed yard and used to make the roofs of buildings waterproof."),
      -- TRANSLATORS: Helptext for a ware: Reed
      frisians = pgettext("frisians_ware", "Reed is grown in a reed farm. Nothing is better suited to make roofs waterproof. It is also used to make buckets and fishing nets as well as cloth." );
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
