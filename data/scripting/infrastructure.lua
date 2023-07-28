-- RST
-- infrastructure.lua
-- ------------------
--
-- This script contains function that ease setting up an initial infrastructure
-- for maps. This includes placing buildings and roads.
--
-- To make these functions available include this file at the beginning
-- of a script via:
--
-- .. code-block:: lua
--
--    include "scripting/infrastructure.lua"
--

-- RST
-- .. function:: connected_road(roadtype, plr, sflag, descr[, create_carriers = true])
--
--    This is a convenience function to create roads and waterways. It is basically a frontend
--    to :meth:`wl.bases.PlayerBase.place_road`. The road is forced, that is other
--    stuff in the way is removed. A sample usage:
--
--    .. code-block:: lua
--
--       local game = wl.Game()
--       field = game.map:get_field(20, 20)
--       building = game.players[1]:place_building("barbarians_sentry", field, true, true)
--       connected_road("normal", game.players[1], building.flag, "r,r|br,r|r,r")
--
--    After placing a building this would create a road starting from the
--    building’s flag and goes from there 2 steps right (east), places a new
--    flag. Then it goes one step bottom-right and one step right, places the
--    next flag and then goes two steps right again and places the last flag.
--    If a flag at this last position already exists, the road gets connected
--    to the existing flag.
--
--    :arg roadtype: :const:`"normal"`, :const:`"busy"`, or :const:`"waterway"`
--    :type roadtype: :class:`string`
--    :arg plr: The player for which the road is created
--    :type plr: :class:`wl.game.Player`
--    :arg sflag: The starting flag for this road.
--    :type sflag: :class:`wl.map.Flag`
--    :arg descr: The description of this road. This are comma separated
--       directions as those accepted by :meth:`wl.bases.PlayerBase.place_road`. A
--       :const:`|` defines a flag.
--    :arg create_carriers: If this is :const:`true` carriers are created for
--       the roads. Otherwise no carriers will be created.
--    :type create_carriers: :class:`boolean`

function connected_road(roadtype, p, start, cmd, g_create_carriers)
   create_carriers = true
   if g_create_carriers ~= nil then
      create_carriers = g_create_carriers
   end

   if cmd:sub(-1) ~= "|" then
      cmd = cmd .. "|"
   end

   moves = {}
   for m in cmd:gmatch("%a+[,|]") do
      moves[#moves+1] = m:sub(1,-2)
      if(m:sub(-1) == '|') then
         moves[#moves+1] = true -- Force the road
         r = p:place_road(roadtype, start, table.unpack(moves))
         start = r.end_flag
         if create_carriers then
            r:set_workers(r.valid_workers)
         end
         moves = {}
      end
   end
end

-- RST
-- .. function:: prefilled_buildings(plr, b1_descr[, b2_descr, ...])
--
--    Create pre-filled buildings. Each description is an array which contains
--    building type, x and y coordinates and pre-fill information.
--    A sample usage:
--
--    .. code-block:: lua
--
--       prefilled_buildings(wl.Game().players[1],
--          {"empire_sentry", 57, 9},                               -- Sentry completely full with soldiers
--          {"empire_sentry", 57, 9, soldiers={[{0,0,0,0}]=1}},     -- Sentry with one soldier
--          {"empire_bakery", 55, 20, inputs = {wheat=6, water=6}}, -- bakery with wares and workers
--          {"empire_well", 52, 30},                                -- a well with workers
--       )
--
--    :arg plr: The player for which the building is created
--    :type plr: :class:`wl.game.Player`
--    :arg b1_descr: An :const:`array` of tables. Each table must contain at
--       least the name of the building, and the x and y position of the field
--       where the building should be created. Optional entries are:
--
--       wares
--          A table of {:const:`"ware_name"`,count} as expected by
--          :meth:`wl.map.HasWares.set_wares`. This is valid for
--          :class:`wl.map.Warehouse` and must not be used otherwise.
--       inputs
--          A table of {:const:`"name"`,count} as expected by
--          :meth:`wl.map.HasInputs.set_inputs`. Inputs are wares or workers
--          which are consumed by the building. This is valid for
--          :class:`wl.map.ProductionSite` and must not be used otherwise.
--       soldiers
--          A table of {{soldier_descr},count} as expected by
--          :meth:`wl.map.HasSoldiers.set_soldiers`.  If this is nil, the site
--          will be filled with {0,0,0,0} soldiers.
--       workers
--          A table of {:const:`"worker_name"`,count} as expected by
--          :meth:`wl.map.HasWorkers.set_workers`.  Note that ProductionSites
--          are filled with workers by default.
--    :type b1_descr: :class:`array`
--
--    :returns: A table of created :class:`buildings <wl.map.Building>`

function prefilled_buildings(p, ...)
   local b_table = {}
   for idx,bdescr in ipairs({...}) do
      local b = p:place_building(bdescr[1], wl.Game().map:get_field(bdescr[2],bdescr[3]), false, true)
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
      if bdescr.inputs then b:set_inputs(bdescr.inputs) end

      table.insert(b_table, b)
   end
   return b_table
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
--                 See: :meth:`wl.map.Field.region`
--    :type region: :class:`array`
--    :arg opts:  A table with prefill information (wares, soldiers, workers,
--                see :func:`prefilled_buildings`)
--    :type opts: :class:`table`
--
--    :returns: The :class:`wl.map.Building` created

function place_building_in_region(plr, building, fields, gargs)
   push_textdomain("widelands")
   local idx
   local f
   local args = gargs or {}

   while #fields > 0 do
      local idx = math.random(#fields)
      local f = fields[idx]

      if plr:get_suitability(building, f) then
         args[1] = building
         args[2] = f.x
         args[3] = f.y
         return prefilled_buildings(plr, args)[1]
      end
      table.remove(fields, idx)
   end
   plr:send_to_inbox(
      -- TRANSLATORS: Short for "Not enough space"
      _("No Space"),
      p(_([[Some of your starting buildings didn’t have enough room and weren’t built. You are at a disadvantage with this; consider restarting this map with a fair starting condition.]])),
      {popup=true, heading=_("Not enough space")}
   )
   pop_textdomain()
end


-- RST
-- .. function:: is_building(immovable)
--
--    Checks whether an immovable is a finished building, i.e. not
--    a construction site.
--
--    :arg immovable: The immovable to test
--
--    :returns: :const:`true` if the immovable is a building
--
--    Example:
--
--    .. code-block:: lua
--
--       field = wl.Game().map:get_field(20, 45)
--       if field.immovable then
--          if is_building(field.immovable) then
--             -- do something ..
--

function is_building(immovable)
   return immovable.descr.type_name == "productionsite" or
      immovable.descr.type_name == "warehouse" or
      immovable.descr.type_name == "militarysite" or
      immovable.descr.type_name == "trainingsite" or
      immovable.descr.type_name == "market"
end

-- RST
-- .. function:: add_wares_to_warehouse(player, warehouse, waretable)
--
--    Adds(subtracts) wares to the first warehouse of specified type
--
--    :arg player: the player to add wares to
--    :type player: :class:`wl.game.Player`
--    :arg warehouse: The type of warehouse to add wares to, e.g. :const:`"empire_warehouse"`
--    :type warehouse: :const:`"string"`
--    :arg waretable: a table of pairs {:const:`"warename"` = value}
--    :type waretable: :class:`table`

function add_wares_to_warehouse(player, warehouse, waretable)
   local hq = player:get_buildings(warehouse)[1]
   for ware,warecount in pairs(waretable) do
      local oldwarecount = hq:get_wares(ware) or 0
      if warecount < 0 and -warecount > oldwarecount then
         warecount = -oldwarecount
      end
      hq:set_wares(ware, oldwarecount+warecount)
   end
end

-- RST
-- .. function:: check_trees_rocks_poor_hamlet(player, sf, warehouse, waretable_rocks, waretable_trees)
--
--    Used for starting condition "Poor Hamlet". Checks for rocks or trees in
--    the region of the player starting field and adds wares to the warehouse
--    if no immovables were found.
--
--    :arg player: the player to check
--    :type player: :class:`wl.game.Player`
--    :arg sf: starting field of the player
--    :type sf: :attr:`starting_field`
--    :arg warehouse: The type of warehouse to add wares to
--    :type warehouse: :const:`string`
--    :arg waretable_rocks: a table of pairs {:const:`"name"` = value} to add if no rocks were found nearby
--    :type waretable_rocks: :const:`table`
--    :arg waretable_trees: a table of pairs {:const:`"name"` = value} to add if no trees were found nearby
--    :type waretable_trees: :const:`table`
--    :arg min_trees: minimal number of trees (optional)
--    :type min_trees: :class:`integer`

function check_trees_rocks_poor_hamlet(player, sf, warehouse, waretable_rocks, waretable_trees, min_trees)
   if not min_trees then -- pass argument if this default does not match
      min_trees = waretable_trees.log + (waretable_trees.planks or 0) * 2
   end
   -- NOTE: pessimistically, this could be a single rock
   local has_rocks = false
   local n_trees = 0
   for k,f in pairs(sf:region(10)) do
      if f.immovable then
         if not has_rocks and f.immovable:has_attribute('rocks') then
            has_rocks = true
         elseif n_trees < min_trees and f.immovable:has_attribute('tree') then
            n_trees = n_trees + 1
         end
         if has_rocks and n_trees >= min_trees then
            break
         end
      end
   end
   if not has_rocks then
      add_wares_to_warehouse(player, warehouse, waretable_rocks)
      player:send_to_inbox(_("No rocks nearby"), _("There are no rocks near to your starting position. Therefore, you receive extra resources for bootstrapping your economy."))
   end
   -- adding exactly one forester
   if n_trees < min_trees then
      waretable_trees.log = math.max(0, waretable_trees.log - n_trees) -- reduce additional logs if there are trees
      add_wares_to_warehouse(player, warehouse, waretable_trees)
      player:send_to_inbox(_("Not enough trees nearby"), _("There are not enough trees near to your starting position. Therefore, you receive extra resources for bootstrapping your economy."))
   end
end
