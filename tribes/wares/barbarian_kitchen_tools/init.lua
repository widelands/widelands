dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_kitchen_tools",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Kitchen Tools",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"kitchen tools",
   tribe = "barbarians",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"How can one create a meal, snack or ration, if there are no kitchen tools? Be sure to have a metal workshop to produce this basic tool (but it ceases to be produced by the building if it is enhanced to an axfactory and war mill).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 8 },
      },
   }
}
