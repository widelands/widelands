dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_smoked_fish",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Smoked Fish",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"smoked fish",
   tribe = "atlanteans",
   default_target_quantity = 30,
   preciousness = 3,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"As no Atlantean likes raw fish, smoking it in a smokery is the most common way to make it edible.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 16 },
      },
   }
}
