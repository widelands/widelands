dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_meat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Meat",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"meat",
   tribe = "atlanteans",
   default_target_quantity = 20,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Meat is one food resource of the Atlanteans. It is obtained from wild game taken by the hunter and has to be smoked in a smokery before being delivered to mines and training sites (dungeon and labyrinth).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 9 },
      },
   }
}
