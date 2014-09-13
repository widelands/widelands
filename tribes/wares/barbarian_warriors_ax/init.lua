dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_warriors_ax",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Warrior’s Ax",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"warrior’s axes",
   tribe = "barbarians",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The warrior’s axe is the most dangerous of all Barbarian weapons. Only a few warriors ever were able to handle those huge and powerful axes. It is produced in a war mill and used – together with food – in a training camp to train soldiers from attack level 4 to level 5.
",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 7 },
      },
   }
}
