-- RST
-- animations.lua
-- --------------
--
-- This script contain some animations to reveal and hide fields seen 
-- by a player. This functions are currently used in the campaigns and scenarios
-- to tell the prologue to a story.

-- RST
-- .. function:: reveal_randomly(player, region, time)
--
--    Reveal a given region field by field, where the fields 
--    are chosen randomly. The region get hidden prior revealing.
--    The animation runs the specified time regardless how big the given
--    region is. So region(6) and region(13) will take the same time.
--    See also :meth:`wl.map.Field.region`
--
--    :arg player: The player who get sight to the region
--    :arg region: The region that has to be revealed
--    :arg time: Optional. The time the whole animation will run. 
--               Defaults to 1000 (1 sec)

function reveal_randomly(plr, region, time)
   local buildhelp_state = wl.ui.MapView().buildhelp
   plr:hide_fields(region, true)

   if buildhelp_state then
      -- Turn off buildhelp during animation
      wl.ui.MapView().buildhelp = false
   end
   -- If no 'time' is given the default 1000 is used
   if not time then time = 1000 end

   -- Calculate the sleep as integer
   local delay = math.floor(time / #region)
   -- Make sure 'delay' is valid
   if delay < 1 then delay = 1 end

   -- Reveal field by field
   while #region > 0 do
      local id = math.random(1, #region)
      plr:reveal_fields({region[id]})
      sleep(delay)
      table.remove(region, id)
   end
   wl.ui.MapView().buildhelp = buildhelp_state
end

-- RST
-- .. function:: hide_randomly(player, region, time)
--
--    Hide a given region field by field, where the fields 
--    are chosen randomly. The animation runs the specified time regardless
--    how big the given region is. So region(6) and region(13) will take
--    the same time. See also :meth:`wl.map.Field.region`
--
--    :arg player: The player who's sight get hidden
--    :arg region: The region that has to be hidden
--    :arg time: Optional. The time the whole animation will run. 
--               Defaults to 1000 (1 sec)

function hide_randomly(plr, region, time)
   -- Turn off buildhelp
   wl.ui.MapView().buildhelp = false
   if not time then time = 1000 end

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
-- .. function:: reveal_concentric(player, center, max_radius, delay)
--
--    Reveal a part of the map in a concentric way beginning from center onto
--    max_radius. The region get hidden prior revealing.
--
--    :arg player: The player who get sight to the region
--    :arg center: The field from where the animation should start revealing
--    :arg max_radius: The last ring to reveal
--    :arg delay: Optional, defaults to 100. The delay between each ring is 
--                revealed

function reveal_concentric(plr, center, max_radius, delay)
   local buildhelp_state = wl.ui.MapView().buildhelp
   if buildhelp_state then
      -- Turn off buildhelp during animation
      wl.ui.MapView().buildhelp = false
   end
   plr:hide_fields(center:region(max_radius), true)
   if not delay then delay = 100 end
   local steps = 0
   while steps < max_radius do
      plr:reveal_fields(center:region(steps))
      steps = steps + 1
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
--    :arg player: The player who's sight get hidden
--    :arg center: The field where the animation should end hiding
--    :arg max_radius: The first ring to hide
--    :arg delay: Optional, defaults to 100. The delay between each ring is 
--                revealed

function hide_concentric(plr, center, max_radius, delay)
   -- Turn off buildhelp
   wl.ui.MapView().buildhelp = false
   if not delay then delay = 100 end
   while max_radius > 0 do
      local to_hide = center:region(max_radius, max_radius - 1)
      plr:hide_fields(to_hide, true)
      sleep(delay)
      max_radius = max_radius -1
   end
   -- Hide the remaining field
   plr:hide_fields({center},true)
end
