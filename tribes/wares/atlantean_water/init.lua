dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_water",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Water",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"water",
   tribe = "atlanteans",
   default_target_quantity = 25,
   preciousness = 7,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Water is the essence of life! It is used in the bakery and the horse and spider farms.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 8 },
      },
   }
}
