dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_snack",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Snack",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"snacks",
   tribe = "barbarians",
   default_target_quantity = 15,
   preciousness = 5,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"A bigger morsel than the ration to provide miners in deep mines. It is produced in an inn or a big inn out of fish/meat, pitta bread and beer.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 8 },
      },
   }
}
