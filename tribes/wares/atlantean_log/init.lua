dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_log",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Log",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"logs",
   tribe = "atlanteans",
   default_target_quantity = 40,
   preciousness = 14,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Logs are the base for planks, which are used in nearly every building of the Atlanteans. Besides the sawmill, the charcoal kiln, the toolsmithy and the smokery also need logs for their work.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 13 },
      },
   }
}
