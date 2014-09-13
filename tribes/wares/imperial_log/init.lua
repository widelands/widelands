dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_log",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Log",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"logs",
   tribe = "empire",
   default_target_quantity = 40,
   preciousness = 14,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Logs are an important building material for the Empire. The lumberjacks fell the trees; foresters care for the supply of trees. Logs are also used by the charcoal kiln, the toolsmithy and the sawmill.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 7 },
      },
   }
}
