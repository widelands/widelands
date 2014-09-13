dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_coal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Coal",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"coal",
   tribe = "barbarians",
   default_target_quantity = 20,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The fires of the barbarians are usually fed with coal. It is produced in coal mines or out of logs in a charcoal kiln. Consumers are several buildings: lime kiln, smelting works, axfactory, war mill, and helm smithy.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 6 },
      },
   }
}
