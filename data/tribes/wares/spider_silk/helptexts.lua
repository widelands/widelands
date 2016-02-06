function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Spider Silk
      atlanteans = pgettext("atlanteans_ware", "Spider silk is produced by spiders, which are bred by spider farms. It is processed into spidercloth in a weaving mill.")
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
