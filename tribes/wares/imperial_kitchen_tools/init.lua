dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_kitchen_tools",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Kitchen Tools",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"kitchen tools",
   tribe = "empire",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"How can one create a ration or meal if there are no kitchen tools? They are produced in a toolsmithy and used in taverns and inns.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 8 },
      },
   }
}
