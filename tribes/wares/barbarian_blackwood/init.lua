dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_blackwood",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Blackwood",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"blackwood",
   tribe = "barbarians",
   default_target_quantity = 40,
   preciousness = 10,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This fire-hardened wood is as hard as iron and it is used for several buildings. It is produced out of logs in the wood hardener.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 10 },
      },
   }
}
