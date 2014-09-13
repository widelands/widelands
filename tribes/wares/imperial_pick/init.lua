dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_pick",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Pick",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"picks",
   tribe = "empire",
   default_target_quantity = 2,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This pick is used by the stonemasons and the miners. It is produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 4 },
      },
   }
}
