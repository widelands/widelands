function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Spidercloth
      atlanteans = pgettext("atlanteans_ware", "Spidercloth is made out of spider silk in a weaving mill. It is used in the toolsmithy and the shipyard. Also some higher developed buildings need spidercloth for their construction.")
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
