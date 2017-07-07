function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Frisian Bread
      frisians = pgettext("frisians_ware", "Bread is made out of barley and water and is used in the taverns and drinking halls to prepare rations. It is also consumed by training soldiers.")
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
