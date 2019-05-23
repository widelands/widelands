dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "atlanteans_resi_none",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "No Resources"),
   helptext_script = dirname .. "../helptexts/none.lua",
   icon = dirname .. "pics/none_0.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/none_?.png"),
         hotspot = {10, 36},
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "atlanteans_resi_water",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Water"),
   helptext_script = dirname .. "../helptexts/water.lua",
   icon = dirname .. "pics/water_0.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/water_?.png"),
         hotspot = {10, 36},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "atlanteans_resi_coal_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Coal"),
   helptext_script = dirname .. "../helptexts/coal_1.lua",
   icon = dirname .. "pics/coal_1_0.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/coal_1_?.png"),
         hotspot = {10, 36},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "atlanteans_resi_gold_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Gold"),
   helptext_script = dirname .. "../helptexts/gold_1.lua",
   icon = dirname .. "pics/gold_1_0.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/gold_1_?.png"),
         hotspot = {10, 36},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "atlanteans_resi_iron_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Iron"),
   helptext_script = dirname .. "../helptexts/iron_1.lua",
   icon = dirname .. "pics/iron_1_0.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/iron_1_?.png"),
         hotspot = {10, 36},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "atlanteans_resi_stones_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Precious Stones"),
   helptext_script = dirname .. "../helptexts/stones_1.lua",
   icon = dirname .. "pics/stones_1_0.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/stones_1_?.png"),
         hotspot = {10, 36},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "atlanteans_resi_coal_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Coal"),
   helptext_script = dirname .. "../helptexts/coal_2.lua",
   icon = dirname .. "pics/coal_2_0.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/coal_2_?.png"),
         hotspot = {10, 36},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "atlanteans_resi_gold_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Gold"),
   helptext_script = dirname .. "../helptexts/gold_2.lua",
   icon = dirname .. "pics/gold_2_0.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/gold_2_?.png"),
         hotspot = {10, 36},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "atlanteans_resi_iron_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Iron"),
   helptext_script = dirname .. "../helptexts/iron_2.lua",
   icon = dirname .. "pics/iron_2_0.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/iron_2_?.png"),
         hotspot = {10, 36},
         fps = 4,
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "atlanteans_resi_stones_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Precious Stones"),
   helptext_script = dirname .. "../helptexts/stones_2.lua",
   icon = dirname .. "pics/stones_2_0.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pics/stones_2_?.png"),
         hotspot = {10, 36},
         fps = 4,
      },
   }
}
