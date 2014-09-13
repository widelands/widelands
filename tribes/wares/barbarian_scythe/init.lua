dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_scythe",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Scythe",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"scythes",
   tribe = "barbarians",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The scythe is the tool of the farmer. It is produced in the metal workshop (but it ceases to be produced by the building if it is enhanced to an axfactory and war mill).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 2 },
      },
   }
}
