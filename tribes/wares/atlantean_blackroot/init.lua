dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_blackroot",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Blackroot",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"blackroots",
   tribe = "atlanteans",
   default_target_quantity = 20,
   preciousness = 10,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Blackroots are a special kind of root produced in blackroot farms and processed in mills. The Atlanteans like their strong taste and use their flour for making bread.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 15 },
      },
   }
}
