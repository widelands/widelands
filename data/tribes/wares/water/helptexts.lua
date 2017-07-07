function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Water
      default = pgettext("ware", "Water is the essence of life!"),
      -- TRANSLATORS: Helptext for a ware: Water
      atlanteans = pgettext("atlanteans_ware", "Water is used in the bakery and the horse and spider farms."),
      -- TRANSLATORS: Helptext for a ware: Water
      frisians = pgettext("frisians_ware", "Water is used to bake bread and brew beer. Reindeer Farms and Aqua Farms also consume it."),
      -- TRANSLATORS: Helptext for a ware: Water
      barbarians = pgettext("barbarians_ware", "Water is used in the bakery, the micro brewery and the brewery. The lime kiln and the cattle farm also need to be supplied with water."),
      -- TRANSLATORS: Helptext for a ware: Water
      empire =  pgettext("empire_ware", "Water is used in the bakery and the brewery. The donkey farm, the sheep farm and the piggery also need to be supplied with water.")
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
