function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Wheat
      default = pgettext("ware", "Wheat is essential for survival."),
      -- TRANSLATORS: Helptext for a ware: Wheat
      barbarians = pgettext("barbarians_ware", "Wheat is produced by farms and consumed by bakeries, micro breweries and breweries. Cattle farms also need to be supplied with wheat."),
      -- TRANSLATORS: Helptext for a ware: Wheat
      empire = _"Wheat is produced by farms and used by mills and breweries. Donkey farms, sheep farms and piggeries also need to be supplied with wheat."
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
