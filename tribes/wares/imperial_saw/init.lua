dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_saw",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Saw",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"saws",
   tribe = "empire",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This saw is needed by the sawyer. It is produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 7 },
      },
   }
}
