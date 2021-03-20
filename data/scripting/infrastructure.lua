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
--    include "scritping/infrastructure.lua"
--

-- RST
-- .. function:: connected_road(roadtype, plr, sflag, descr[, create_carriers = true])
--
--    This is a convenience function to create roads and waterways. It is basically a frontend
--    to :meth:`wl.game.Player.place_road`. The road is forced, that is other
--    stuff in the way is removed. A sample usage:
--
--    .. code-block:: lua
--
--       local game = wl.Game()
--       connected_road("normal", game.players[1], game.map:get_field(20,20).immovable, "r,r|br,r|r,r")
--
--    This would create a road starting from the Flag standing at field(20,20)
--    which must exist and goes from there 2 steps right (east), places a new
--    flag. Then it goes one step bottom-right and one step right, places the
--    next flag and then goes two steps right again and places the last flag.
--
--    :arg roadtype: 'normal', 'busy', or 'waterway'
--    :type roadtype: :class:`string`
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
            if roadtype == "normal" then r:set_workers(p.tribe.carrier, 1)
            elseif roadtype == "busy" then r:set_workers({[p.tribe.carrier] = 1, [p.tribe.carrier2] = 1})
            else r:set_workers(p.tribe.ferry, 1) end
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
--          {"bakery", 55, 20, inputs = {wheat=6, water=6}}, -- bakery with wares and workers
--          {"well", 52, 30}, -- a well with workers
--       )
--
--    :arg plr: The player for which the building is created
--    :type plr: :class:`wl.game.Player`
--    :arg b1_descr: An array of tables. Each table must contain at
--       least the name of the building, and the x and y positions of the field
--       where the building should be created. Optional entries are:
--
--       wares
--          A table of (name,count) as expected by
--          :meth:`wl.map.Warehouse.set_wares`. This is valid for
--          :class:`wl.map.Warehouse` and must not be used otherwise.
--       inputs
--          A table of (name,count) as expected by
--          :meth:`wl.map.ProductionSite.set_inputs`. Inputs are wares or workers
--          which are consumed by the building. This is valid for
--          :class:`wl.map.ProductionSite` and must not be used otherwise.
--       soldiers
--          A table of (soldier_descr,count) as expected by
--          :meth:`wl.map.HasSoldiers.set_soldiers`.  If this is nil, the site
--          will be filled with {0,0,0,0} soldiers.
--       workers
--          A table of (workers_name,count) as expected by
--          :meth:`wl.map.Warehouse.set_workers`.  Note that ProductionSites
--          are filled with workers by default.
--    :type b1_descr: :class:`array`
--
--    :returns: A table of created buildings

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
--    :type region: :class:`array`
--    :arg opts:  A table with prefill information (wares, soldiers, workers,
--       see :func:`prefilled_buildings`)
--    :type opts: :class:`table`
--
--    :returns: The building created

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
      _"No Space",
      p(_([[Some of your starting buildings didn’t have enough room and weren’t built. You are at a disadvantage with this; consider restarting this map with a fair starting condition.]])),
      {popup=true, heading=_"Not enough space"}
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
--    :returns: true if the immovable is a building

function is_building(immovable)
   return immovable.descr.type_name == "productionsite" or
      immovable.descr.type_name == "warehouse" or
      immovable.descr.type_name == "militarysite" or
      immovable.descr.type_name == "trainingsite"
end

-- RST
-- .. function:: add_wares_to_warehouse(player, warehouse, waretable)
--
--    Adds(subtracts) wares to the first warehouse of specified type
--
--    :arg player: the player to add wares to
--    :arg warehouse: The type of warehouse (string) to add wares to
--    :arg waretable: a table of pairs {ware = value}

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
--    Checks for rocks or trees in the region of the player starting field and adds wares if no immovables found
--
--    :arg player: the player to check
--    :arg sf: starting field of the player
--    :arg warehouse: The type of warehouse (string) to add wares to
--    :arg waretable_rocks: a table of pairs {ware = value} to add if no rocks
--    :arg waretable_trees: a table of pairs {ware = value} to add if no trees

function check_trees_rocks_poor_hamlet(player, sf, warehouse, waretable_rocks, waretable_trees)
   -- NOTE: pessimistically, this could be a single rock and a single tree
   local has_rocks = false
   local has_trees = false
   for k,f in pairs(sf:region(10)) do
      if f.immovable then
         if not has_rocks and f.immovable:has_attribute('rocks') then
            has_rocks = true
         elseif not has_trees and f.immovable:has_attribute('tree') then
            has_trees = true
         end
         if has_trees and has_rocks then
            break
         end
      end
   end
   if not has_rocks then
      add_wares_to_warehouse(player, warehouse, waretable_rocks)
      player:send_to_inbox(_"No rocks nearby", _"There are no rocks near to your starting position. Therefore, you receive extra resources for bootstrapping your economy.")
   end
   -- adding exactly one forester
   if not has_trees then
      add_wares_to_warehouse(player, warehouse, waretable_trees)
      player:send_to_inbox(_"No trees nearby", _"There are no trees near to your starting position. Therefore, you receive extra resources for bootstrapping your economy.")
   end
end
