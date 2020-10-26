push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_immovable_type {
   name = "atlanteans_resi_none",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "No Resources"),
   icon = dirname .. "pics/none.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "pics",
         basename = "none",
         hotspot = {10, 36},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

descriptions:new_immovable_type {
   name = "atlanteans_resi_water",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Water"),
   icon = dirname .. "pics/water.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "pics",
         basename = "water",
         hotspot = {10, 36},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

descriptions:new_immovable_type {
   name = "atlanteans_resi_coal_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Coal"),
   icon = dirname .. "pics/coal_few.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "pics",
         basename = "coal_few",
         hotspot = {10, 36},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

descriptions:new_immovable_type {
   name = "atlanteans_resi_gold_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Gold"),
   icon = dirname .. "pics/gold_few.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "pics",
         basename = "gold_few",
         hotspot = {10, 36},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

descriptions:new_immovable_type {
   name = "atlanteans_resi_iron_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Iron"),
   icon = dirname .. "pics/iron_few.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "pics",
         basename = "iron_few",
         hotspot = {10, 36},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

descriptions:new_immovable_type {
   name = "atlanteans_resi_stones_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Crystals"),
   icon = dirname .. "pics/stone_few.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "pics",
         basename = "stone_few",
         hotspot = {10, 36},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

descriptions:new_immovable_type {
   name = "atlanteans_resi_coal_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Coal"),
   icon = dirname .. "pics/coal_much.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "pics",
         basename = "coal_much",
         hotspot = {10, 36},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

descriptions:new_immovable_type {
   name = "atlanteans_resi_gold_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Gold"),
   icon = dirname .. "pics/gold_much.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "pics",
         basename = "gold_much",
         hotspot = {10, 36},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

descriptions:new_immovable_type {
   name = "atlanteans_resi_iron_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Iron"),
   icon = dirname .. "pics/iron_much.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "pics",
         basename = "iron_much",
         hotspot = {10, 36},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

descriptions:new_immovable_type {
   name = "atlanteans_resi_stones_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Crystals"),
   icon = dirname .. "pics/stone_much.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   spritesheets = {
      idle = {
         directory = dirname .. "pics",
         basename = "stone_much",
         hotspot = {10, 36},
         frames = 4,
         columns = 2,
         rows = 2
      }
   }
}

pop_textdomain()
