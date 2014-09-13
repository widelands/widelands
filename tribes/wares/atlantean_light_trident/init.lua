dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_light_trident",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Light Trident",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"light tridents",
   tribe = "atlanteans",
   default_target_quantity = 30,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This is the basic weapon of the Atlantean soldiers. Together with a tabard, it makes up the equipment of young soldiers. Light tridents are produced in the weapon smithy as are all other tridents.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
