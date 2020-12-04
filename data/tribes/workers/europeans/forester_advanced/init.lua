push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_forester_advanced",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Advanced Forester"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   programs = {
      plant = {
         "findspace=size:any radius:12 avoid:field saplingsearches:12",
         "walk=coords",
         "animate=dig duration:2s500ms",
         "animate=planting duration:1s500ms",
         "plant=attrib:tree_sapling",
         "animate=water duration:2s",
         "return"
      },
      check = {
         "findspace=size:any radius:12 terraform",
      },
      terraform = {
         "findspace=size:any radius:12 terraform",
         "walk=coords",
         "animate=dig duration:2s",
         "terraform",
         "animate=dig duration:2s",
         "return"
      },
   },

   animations = {
      idle = {
         hotspot = { 3, 23 }
      },
      dig = {
         hotspot = { 5, 22 },
         fps = 5
      },
      planting = {
         basename = "plant",
         hotspot = { 18, 23 },
         fps = 10
      },
      water = {
         hotspot = { 18, 26 },
         fps = 5
      },
      walk = {
         hotspot = { 10, 23 },
         fps = 10,
         directional = true
      },
      walkload = {
         basename = "walk",
         hotspot = { 10, 23 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
