dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_steel_trident",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Steel Trident",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"steel tridents",
   tribe = "atlanteans",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This is the medium trident. It is produced in the weapon smithy and used by advanced soldiers in the dungeon to train from attack level 1 to level 2 (together with food). ",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
