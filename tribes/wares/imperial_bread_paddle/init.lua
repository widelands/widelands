dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_bread_paddle",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Bread Paddle",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"bread paddles",
   tribe = "empire",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The bread paddle is the tool of the baker, each baker needs one. It is produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 6 },
      },
   }
}
