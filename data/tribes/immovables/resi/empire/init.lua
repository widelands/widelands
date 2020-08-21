dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "empire_resi_none",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "No Resources"),
   icon = dirname .. "pics/none_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "none",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "empire_resi_water",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Water"),
   icon = dirname .. "pics/water_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "water",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "empire_resi_coal_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Coal"),
   icon = dirname .. "pics/coal_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "coal_few",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "empire_resi_gold_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Gold"),
   icon = dirname .. "pics/gold_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "gold_few",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "empire_resi_iron_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Iron"),
   icon = dirname .. "pics/iron_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "iron_few",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "empire_resi_stones_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Marble"),
   icon = dirname .. "pics/stone_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "stone_few",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "empire_resi_coal_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Coal"),
   icon = dirname .. "pics/coal_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "coal_much",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "empire_resi_gold_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Gold"),
   icon = dirname .. "pics/gold_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "gold_much",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "empire_resi_iron_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Iron"),
   icon = dirname .. "pics/iron_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "iron_much",
         hotspot = {9, 19}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "empire_resi_stones_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Marble"),
   icon = dirname .. "pics/stone_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         directory = dirname .. "pics",
         basename = "stone_much",
         hotspot = {9, 19}
      }
   }
}
