dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barbarians_resi_none",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: None"),
   helptext_script = dirname .. "../helptexts/none.lua",
   icon = dirname .. "pics/none.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = { dirname .. "pics/none.png" },
         hotspot = {9, 28},
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barbarians_resi_water",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Water Vein"),
   helptext_script = dirname .. "../helptexts/water.lua",
   icon = dirname .. "pics/water.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = { dirname .. "pics/water.png" },
         hotspot = {9, 28},
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barbarians_resi_coal_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Coal Vein"),
   helptext_script = dirname .. "../helptexts/coal_1.lua",
   icon = dirname .. "pics/coal_1.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = { dirname .. "pics/coal_1.png" },
         hotspot = {9, 28},
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barbarians_resi_gold_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Gold Vein"),
   helptext_script = dirname .. "../helptexts/gold_1.lua",
   icon = dirname .. "pics/gold_1.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = { dirname .. "pics/gold_1.png" },
         hotspot = {9, 28},
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barbarians_resi_iron_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Iron Vein"),
   helptext_script = dirname .. "../helptexts/iron_1.lua",
   icon = dirname .. "pics/iron_1.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = { dirname .. "pics/iron_1.png" },
         hotspot = {9, 28},
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barbarians_resi_stones_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Some Stones"),
   helptext_script = dirname .. "../helptexts/stones_1.lua",
   icon = dirname .. "pics/stones_1.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = { dirname .. "pics/stones_1.png" },
         hotspot = {9, 28},
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barbarians_resi_coal_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Main Coal Vein"),
   helptext_script = dirname .. "../helptexts/coal_2.lua",
   icon = dirname .. "pics/coal_2.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = { dirname .. "pics/coal_2.png" },
         hotspot = {9, 28},
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barbarians_resi_gold_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Main Gold Vein"),
   helptext_script = dirname .. "../helptexts/gold_2.lua",
   icon = dirname .. "pics/gold_2.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = { dirname .. "pics/gold_2.png" },
         hotspot = {9, 28},
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barbarians_resi_iron_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: Main Iron Vein"),
   helptext_script = dirname .. "../helptexts/iron_2.lua",
   icon = dirname .. "pics/iron_2.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = { dirname .. "pics/iron_2.png" },
         hotspot = {9, 28},
      },
   }
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "barbarians_resi_stones_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Resources: A Lot of Stones"),
   helptext_script = dirname .. "../helptexts/stones_2.lua",
   icon = dirname .. "pics/stones_2.png",
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         pictures = { dirname .. "pics/stones_2.png" },
         hotspot = {9, 28},
      },
   }
}
