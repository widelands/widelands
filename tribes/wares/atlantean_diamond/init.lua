dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_diamond",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Diamond",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"diamonds",
   tribe = "atlanteans",
   default_target_quantity = 5,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"These wonderful diamonds are used to build some exclusive buildings. They are mined in a crystal mine.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 7 },
      },
   }
}
