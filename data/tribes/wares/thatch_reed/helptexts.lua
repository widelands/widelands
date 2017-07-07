function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Thatch Reed
      barbarians = pgettext("barbarians_ware", "Thatch reed is produced in a reed yard and used to make the roofs of buildings waterproof."),
      -- TRANSLATORS: Helptext for a ware: Thatch Reed
      frisians = pgettext("frisians_ware", "Thatch reed is a special kind of reed that is grown in a reed farm. Nothing is better suited to make roofs waterproof." );
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
