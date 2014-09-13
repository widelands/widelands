dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_shovel",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Shovel",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"shovels",
   tribe = "barbarians",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The shovel is needed for the proper handling of plants. Therefore the gardener and the ranger use it. Produced at the metal workshop (but it ceases to be produced by the building if it is enhanced to an axfactory and war mill).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 5 },
      },
   }
}
