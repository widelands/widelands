dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_ax",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Ax",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"axes",
   tribe = "barbarians",
   default_target_quantity = 30,
   preciousness = 3,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The ax is the basic weapon of the barbarians. All young soldiers are equipped with it.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 7 },
      },
   }
}
