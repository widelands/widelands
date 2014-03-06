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
--       local game = wl.Game()
--       connected_road(game.players[1], game.map:get_field(20,20).immovable, "r,r|br,r|r,r")
--
--    This would create a road starting from the Flag standing at field(20,20)
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
         r = p:place_road(start, table.unpack(moves))
         start = r.end_flag
         if create_carriers then
            r:set_workers("carrier", 1)
         end
         moves = {}
      end
   end
end

-- RST
-- .. function:: prefilled_buildings(plr, b1_descr[, b2_descr, ...])
--
--    Create pre-filled buildings. Each description is a arrays which contain
--    building type, build field and pre-fill information. A sample usage:
--
--    .. code-block:: lua
--
--       prefilled_buildings(wl.Game().players[1],
--          {"sentry", 57, 9}, -- Sentry completely full with soldiers
--          {"sentry", 57, 9, soldier={[{0,0,0,0}]=1}}, -- Sentry with one soldier
--          {"bakery", 55, 20, wares = {wheat=6, water=6}}, -- bakery with wares and workers
--          {"well", 52, 30}, -- a well with workers
--       )
--
--    :arg plr: The player for which the road is created
--    :type plr: :class:`wl.game.Player`
--    :arg b1_descr: An array of tables. Each table must contain at
--       least the name of the building, and the x and y positions of the field
--       where the building should be created. Optional entries are:
--
--       wares
--          A table of (name,count) as expected by
--          :meth:`wl.map.ProductionSite.set_wares`. This is valid for
--          :class:`wl.map.ProductionSite` and :class:`wl.map.Warehouse` and
--          ignored otherwise.
--       soldiers
--          A table of (soldier_descr,count) as expected by
--          :meth:`wl.map.HasSoldiers.set_soldiers`.  If this is nil, the site
--          will be filled with {0,0,0,0} soldiers.
--       workers
--          A table of (workers_name,count) as expected by
--          :meth:`wl.map.Warehouse.set_workers`.  Note that ProductionSites
--          are filled with workers by default.
--    :type b1_descr: :class:`array`
function prefilled_buildings(p, ...)
   for idx,bdescr in ipairs({...}) do
      b = p:place_building(bdescr[1], wl.Game().map:get_field(bdescr[2],bdescr[3]), false, true)
      -- Fill with workers
      if b.valid_workers then b:set_workers(b.valid_workers) end
      if bdescr.workers then b:set_workers(bdescr.workers) end
      -- Fill with soldiers
      if b.max_soldiers and b.set_soldiers then
         if bdescr.soldiers then
            b:set_soldiers(bdescr.soldiers)
         else
            b:set_soldiers({0,0,0,0}, b.max_soldiers)
         end
      elseif bdescr.soldiers then -- Must be a warehouse
         b:set_soldiers(bdescr.soldiers)
      end
      -- Fill with wares if this is requested
      if bdescr.wares then b:set_wares(bdescr.wares) end
   end
end

-- RST
-- .. function:: place_building_in_region(plr, building, region[, opts])
--
--    Tries to place a building randomly in the given region. It places houses
--    using :func:`prefilled_buildings`, therefore uses the same options.
--    If it fails, an error is thrown. This is a most useful function when
--    defining starting conditions (initializations) in normal games.
--
--    :arg plr: The player for which the building is created
--    :type plr: :class:`wl.game.Player`
--    :arg building: The name of the building to create.
--    :type building: :class:`string`
--    :arg region: The fields which are tested for suitability.
--    :type region: :class:`array`
--    :arg opts:  a table with prefill information (wares, soldiers, workers,
--       see :func:`prefilled_buildings`) and the following options:
--
--       req_suitability
--          The reguired suitability for this building. Default value is 1.
--    :type opts: :class:`table`
--
--    :returns: the building created
function place_building_in_region(
   plr, building, fields, gargs
)
   local idx
   local f
   local args = gargs or {}
   local req_suitability = args.req_suitability or 1

   while #fields > 0 do
      local idx = math.random(#fields)
      local f = fields[idx]

      if plr:get_suitability(building, f) >= req_suitability then
         args[1] = building
         args[2] = f.x
         args[3] = f.y
         return prefilled_buildings(plr, args)
      end
      table.remove(fields, idx)
   end
   error(string.format(
      "Could not find a suitable position for building '%s' for player %i",
      building, plr.number)
   )
end
