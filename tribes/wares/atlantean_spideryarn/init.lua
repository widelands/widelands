dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_spideryarn",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Spideryarn",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"spideryarn",
   tribe = "atlanteans",
   default_target_quantity = 10,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This yarn is produced by spiders, which are bred by spider farms. It is processed into spidercloth in a weaving mill.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 16 },
      },
   }
}
