dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_bread_paddle",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Bread Paddle",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"bread paddles",
   tribe = "barbarians",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The bread paddle is the tool of the baker and each baker needs one. It is produced in the metal works like all other tools (but it ceases to be produced by the building if the metal works is enhanced to an axfactory and war mill).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 6 },
      },
   }
}
