dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_iron",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Iron",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"iron",
   tribe = "barbarians",
   default_target_quantity = 20,
   preciousness = 4,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Iron is obtained out of iron ore in the smelting works. It is used to produce weapons and tools in the metal workshop, axfactory, war mill and in the helm smithy.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 9 },
      },
   }
}
