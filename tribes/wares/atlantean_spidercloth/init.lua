dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_spidercloth",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Spidercloth",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"spidercloth",
   tribe = "atlanteans",
   default_target_quantity = 20,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Spidercloth is made out of spideryarn in a weaving mill. It is used in the toolsmithy and the shipyard. Also some higher developed buildings need spidercloth for their construction.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 9 },
      },
   }
}
