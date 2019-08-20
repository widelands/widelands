function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Liana
      amazons = pgettext("amazons_ware", "Lianas grow in the trees. Very durable ropes are made out of it." );
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
