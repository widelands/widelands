dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_fish",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Fish",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"fish",
   tribe = "atlanteans",
   default_target_quantity = 20,
   preciousness = 4,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Fish is one of the biggest food resources of the Atlanteans. It has to be smoked in a smokery before being delivered to mines, training sites and scouts.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 6 },
      },
   }
}
