function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Balsa
      default = pgettext("ware", "Balsa wood an important building material."),
      -- TRANSLATORS: Helptext for a ware: Balsa
      amazons =
         pgettext("amazons_ware", "They are cut by an experienced woodcutter.")
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
