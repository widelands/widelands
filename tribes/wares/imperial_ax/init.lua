dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_ax",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Ax",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"axes",
   tribe = "empire",
   default_target_quantity = 3,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The ax is the tool for the lumberjack. It is produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 3 },
      },
   }
}
