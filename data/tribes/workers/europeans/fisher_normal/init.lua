push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_worker_type {
   name = "europeans_fisher_normal",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Normal Fisher"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   experience = 24,
   becomes = "europeans_fisher_advanced",

   programs = {
      fish_in_sea = {
         "findspace=size:any radius:8 resource:resource_fish",
         "walk=coords",
         "playsound=sound/fisher/fisher_throw_net priority:50% allow_multiple",
         "mine=resource_fish radius:1",
         "animate=fishing duration:8s",
         "playsound=sound/fisher/fisher_pull_net priority:50% allow_multiple",
         "createware=fish",
         "return"
      },
      check_pond = {
         "findobject=attrib:pond_mature radius:8",
      },
      fish_in_pond = {
         "findobject=attrib:pond_mature radius:8",
         "walk=object",
         "animate=fishing duration:8s",
         "callobject=fall_dry",
         "createware=fish",
         "return"
      }
   },

   animations = {
      idle = {
         hotspot = { 2, 20 },
      },
      fishing = {
         hotspot = { 10, 21 },
         fps = 10
      },
      walk = {
         hotspot = { 8, 21 },
         fps = 20,
         directional = true
      },
      walkload = {
         hotspot = { 8, 20 },
         fps = 10,
         directional = true
      }
   }
}

pop_textdomain()
