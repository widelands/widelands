dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_hunting_bow",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Hunting Bow",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"hunting bows",
   tribe = "atlanteans",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This bow is used by the Atlantean hunter. It is produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 16 },
      },
   }
}
