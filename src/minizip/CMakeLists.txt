add_library(wl_minizip STATIC EXCLUDE_FROM_ALL "unzip.cc")

message(WARNING "You are using widelands-bundled minizip sources. Please install your distribution's minizip library or urge your distribution maintainer to include the minizip library in your package repository. Thank you.")
