function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Log
      default = pgettext("ware", "Logs are an important basic building material. They are produced by felling trees."),
      -- TRANSLATORS: Helptext for a ware: Log
      atlanteans = pgettext("atlanteans_ware", "Atlanteans use logs also as the base for planks, which are used in nearly every building. Besides the sawmill, the charcoal kiln, the toolsmithy and the smokery also need logs for their work."),
      -- TRANSLATORS: Helptext for a ware: Log
      barbarians = pgettext("barbarians_ware", "Barbarian lumberjacks fell the trees; rangers take care of the supply of trees. Logs are also used in the metal workshop to build basic tools, and in the charcoal kiln for the production of coal. The wood hardener refines logs into blackwood by hardening them with fire."),
      -- TRANSLATORS: Helptext for a ware: Log
      frisians = pgettext("frisians_ware", "Woodcutters fell the trees; foresters take care of the supply of trees. Logs are also used in the blacksmithy to build basic tools, and in the charcoal kiln for the production of coal. Tanneries use logs as fuel for smoking meat and fish."),
      -- TRANSLATORS: Helptext for a ware: Log
      empire = pgettext("empire_ware", "Imperial lumberjacks fell the trees; foresters take care of the supply of trees. Logs are also used by the charcoal kiln, the toolsmithy and the sawmill.")
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
