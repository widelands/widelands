-- RST
-- infrastructure.lua
-- ------------------
--
-- This script contains function that ease setting up an initial infrastructure
-- for maps. This includes placing buildings and roads.

-- RST
-- .. function:: connected_road(plr, sflag, descr[, create_carriers = true])
--
--    This is a convenience function to create roads. It is basically a frontend
--    to :meth:`wl.game.Player.place_road`. The road is forced, that is other
--    stuff in the way is removed. A sample usage:
--
--    .. code-block:: lua
--       
--       connected_road(wl.game.Player(1), wl.map.Field(20,20).immovable, "r,r|br,r|r,r")
--
--    This would create a road starting from the Flag standing at Field(20,20)
--    which must exist and goes from there 2 steps right (east), places a new
--    flag. Then it goes one step bottom-right and one step right, places the 
--    next flag and then goes two steps right again and places the last flag.
--       
--    :arg plr: The player for which the road is created
--    :type plr: :class:`wl.game.Player`
--    :arg sflag: The starting flag for this road. 
--    :type sflag: :class:`wl.map.Flag`
--    :arg descr: The description of this road. This are comma separated
--       directions as those accepted by :meth:`wl.game.Player.place_road`. A
--       :const:`|` defines a flag.
--    :arg create_carriers: If this is :const:`true` carriers are created for
--       the roads. Otherwise no carriers will be created.
--    :type create_carriers: :class:`boolean`
function connected_road(p, start, cmd, g_create_carriers)
   create_carriers = g_create_carriers or true

   if cmd:sub(-1) ~= "|" then
      cmd = cmd .. "|"
   end

   moves = {}
   for m in cmd:gmatch("%a+[,|]") do 
      moves[#moves+1] = m:sub(1,-2)
      if(m:sub(-1) == '|') then
         moves[#moves+1] = true -- Force the road
         r = p:place_road(start, unpack(moves))
         start = r.end_flag
         if create_carriers then
            r:warp_workers{"carrier"}
         end
         moves = {}
      end
   end
end

-- RST
-- .. function:: prefilled_buildings(plr, buildings_descr)
--
--    Create pre-filled buildings. The description consists of arrays which
--    contain building type, location and pre-filled wares. A sample usage:
--
--    .. code-block:: lua
--
--       prefilled_buildings(wl.game.Player(1),
--          {"sentry", 57, 9}, -- Sentry completely full with soldiers
--          {"bakery", 55, 20, wares = {wheat=6, water=6}}, -- bakery with wares and workers
--          {"well", 52, 30}, -- a well with workers
--       )
--
--    :arg plr: The player for which the road is created
--    :type plr: :class:`wl.game.Player`
--    :arg buildings_descr: An array of tables. Each table must contain at 
--       least the name of the building, and the x and y positions of the field
--       where the building should be created. Optional entries are:
--
--       wares
--          A table of (name,count) as expected by
--          :meth:`wl.map.ProductionSite.set_wares`. This is valid for
--          :class:`wl.map.ProductionSite` and :class:`wl.map.Warehouse` and
--          ignored otherwise. 
--    :type buildings_descr: :class:`array`
function prefilled_buildings(p, ...)
   for idx,bdescr in ipairs({...}) do
      b = p:place_building(bdescr[1], wl.map.Field(bdescr[2],bdescr[3]))
      -- Fill with workers
      if b.valid_workers and b.warp_workers then
         b:warp_workers(b.valid_workers)
      end
      -- Fill with soldiers
      if b.max_soldiers and b.warp_soldiers then
         b:warp_soldiers{[{0,0,0,0}] = b.max_soldiers}
      end
      -- Fill with wares if this is requested
      if bdescr.wares then b:set_wares(bdescr.wares) end
   end
end

