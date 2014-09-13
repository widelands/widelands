dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_pick",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Pick",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"picks",
   tribe = "barbarians",
   default_target_quantity = 2,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This pick is used by the stonemasons and the miners. It is produced in the metal workshop (but it ceases to be produced by the building if it is enhanced to an axfactory and war mill).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 4 },
      },
   }
}
