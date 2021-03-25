-- RST
-- .. _field_animations:
--
-- field_animations.lua
-- --------------------
--
-- This script contains some animations to reveal and hide fields seen
-- by a player. This functions are currently used in the campaigns and scenarios
-- to tell the prologue to a story.
--
-- .. code-block:: lua
--
--    include "scripting/field_animations.lua"
--

-- RST
-- .. function:: reveal_randomly(player, region, time)
--
--    Reveal a given region field by field, where the fields
--    are chosen randomly. The region will be hidden before being revealed.
--    The animation runs the specified time.
--
--    :arg player: The player who gets sight to the region
--    :type player: :class:`wl.game.Player`
--    :arg region: The region that has to be revealed. E.g. by using :meth:`wl.map.Field.region`
--    :type region: :class:`array` of :class:`fields <wl.map.Field>`
--    :arg time: Optional. The time the whole animation will run.
--               Defaults to 1000 (1 sec)
--    :type time: :class:`integer`

function reveal_randomly(plr, region, time)
   -- If no 'time' is given use a default
   time = time or 1000

   -- Make sure the region is hidden
   plr:hide_fields(region, true)

   -- Turn off buildhelp during animation
   local buildhelp_state = wl.ui.MapView().buildhelp
   if buildhelp_state then
      wl.ui.MapView().buildhelp = false
   end

   -- Calculate delay as integer
   local delay = math.floor(time / #region)
   -- Make sure 'delay' is valid
   if delay < 1 then delay = 1 end

   -- Reveal field by field
   while #region > 0 do
      local t = {}
      local id = math.random(1, #region)
      table.insert(t, region[id])
      plr:reveal_fields(t)
      sleep(delay)
      table.remove(region, id)
   end
   -- Restore buildhelp status
   wl.ui.MapView().buildhelp = buildhelp_state
end

-- RST
-- .. function:: hide_randomly(player, region, time)
--
--    Hide a given region field by field, where the fields
--    are chosen randomly. The animation runs the specified time regardless
--    how big the given region is. So region(6) and region(13) will take
--    the same time.
--
--    :arg player: The player whose sight gets hidden
--    :type player: :class:`wl.game.Player`
--    :arg region: The region that will be hidden. E.g. by using :meth:`wl.map.Field.region`
--    :type region: :class:`array` of :class:`fields <wl.map.Field>`
--    :arg time: Optional. The time the whole animation will run.
--               Defaults to 1000 (1 sec)
--    :type time: :class:`integer`

function hide_randomly(plr, region, time)
   time = time or 1000
   -- Turn off buildhelp
   wl.ui.MapView().buildhelp = false

   local delay = math.floor(time / #region)
   if delay < 1 then delay = 1 end
   while #region > 0 do
      local id = math.random(1, #region)
      plr:hide_fields({region[id]},true)
      table.remove(region, id)
      sleep(delay)
   end
end

-- RST
-- .. function:: reveal_concentric(player, center, max_radius, hide, delay)
--
--    Reveal a part of the map in a concentric way beginning from center onto
--    max_radius. The region get hidden prior revealing as default.
--
--    :arg player: The player who gets sight to the region
--    :type player: :class:`wl.game.Player`
--    :arg center: The field from where the animation should start revealing
--    :type center: :class:`wl.map.Field`
--    :arg max_radius: The last ring to reveal
--    :type max_radius: :class:`integer`
--    :arg hide: Optional, if :const:`false` automatic hiding is disabled
--    :type hide: :class:`boolean`
--    :arg delay: Optional, defaults to 100. The delay between revealing each
--                ring. If you want to set the delay, you must also set `hide`
--    :type delay: :class:`integer`

function reveal_concentric(plr, center, max_radius, hide, delay)
   delay = delay or 100
   if hide == nil then hide = true end

   local buildhelp_state = wl.ui.MapView().buildhelp
   if buildhelp_state then
      -- Turn off buildhelp during animation
      wl.ui.MapView().buildhelp = false
   end

   if hide then
      plr:hide_fields(center:region(max_radius), true)
   end

   local radius = 0
   while radius <= max_radius do
      plr:reveal_fields(center:region(radius))
      radius = radius + 1
      sleep(delay)
   end
   wl.ui.MapView().buildhelp = buildhelp_state
end

-- RST
-- .. function:: hide_concentric(player, center, max_radius, delay)
--
--    Hide a part of the map in a concentric way beginning from max_radius onto
--    center.
--
--    :arg player: The player whose sight gets hidden
--    :type player: :class:`wl.game.Player`
--    :arg center: The field where the animation should end hiding
--    :type center: :class:`wl.map.Field`
--    :arg max_radius: The first ring to hide
--    :type max_radius: :class:`integer`
--    :arg delay: Optional, defaults to 100. The delay between revealing each
--                ring
--    :type time: :class:`integer`

function hide_concentric(plr, center, max_radius, delay)
   delay = delay or 100
   -- Turn off buildhelp
   wl.ui.MapView().buildhelp = false
   while max_radius > 0 do
      local to_hide = center:region(max_radius, max_radius - 1)
      plr:hide_fields(to_hide, true)
      sleep(delay)
      max_radius = max_radius -1
   end
   -- Hide the remaining field
   plr:hide_fields({center},true)
end

-- RST
-- .. function:: get_sees_fields(player)
--
--    Gather all fields a player can see in the current view. The current view
--    is the whole area of the map in the current game window. You can use this
--    function to get an unregular (non hexagonal) region and feed e.g.
--    :meth:`reveal_randomly` with it.
--
--    :arg player: The player for whom the fields get gathered
--    :type player: :class:`wl.game.Player`
--    :returns: A table containing all visible fields in the current view

function get_sees_fields(plr)
   local sees_fields = {}
   for x=0, wl.Game().map.width-1 do
      for y=0, wl.Game().map.height-1 do
         f = wl.Game().map:get_field(x,y)
         if wl.ui.MapView():is_visible(f) then
            -- Gather only fields which are seen in the view
            if plr:sees_field(f) then
               table.insert(sees_fields, f)
            end
         end
      end
   end
   return sees_fields
end
