dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_gold_yarn",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Gold Yarn",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"gold yarn",
   tribe = "atlanteans",
   default_target_quantity = 5,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This yarn, made of gold by the gold spinning mill, is used for weaving the exclusive golden tabard in the weaving mill.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 16 },
      },
   }
}
