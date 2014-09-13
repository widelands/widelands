dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_coal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Coal",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"coal",
   tribe = "atlanteans",
   default_target_quantity = 20,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The Atlantean fires in smelting works, armorsmithies and weaponsmithies are fed with coal. This coal is mined in coal mines or produced out of logs by a charcoal kiln.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 6 },
      },
   }
}
