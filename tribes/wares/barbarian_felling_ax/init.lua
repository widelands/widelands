dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_felling_ax",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Felling Ax",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"felling axes",
   tribe = "barbarians",
   default_target_quantity = 5,
   preciousness = 3,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The felling ax is the tool of the lumberjack to take down trees. It is produced in the metal workshop (but it ceases to be produced by the building if it is enhanced to an axfactory and war mill).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 7 },
      },
   }
}
