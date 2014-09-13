dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_log",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Log",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"logs",
   tribe = "barbarians",
   default_target_quantity = 40,
   preciousness = 14,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Logs are a principal building material of the barbarians. The trees are felled by lumberjacks; rangers care for the supply of trees. Logs are also used in the metal workshop to build basic tools, and in the charcoal kiln for the production of coal. The wood hardener refines logs by hardening them in the fire which gives blackwood.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 7 },
      },
   }
}
