dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_pick",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Pick",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"picks",
   tribe = "atlanteans",
   default_target_quantity = 3,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Picks are used by stonecutters and miners. They are produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 4 },
      },
   }
}
