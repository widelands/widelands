dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_stone",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Stone",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"stones",
   tribe = "atlanteans",
   default_target_quantity = 20,
   preciousness = 5,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Stone is a basic building ware of the Atlanteans. It is produced in a quarry or a crystal mine.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 8 },
      },
   }
}
