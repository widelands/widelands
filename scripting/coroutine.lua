-- RST
-- coroutine.lua
-- -------------
--
-- This script contains convenience wrapper around creation and resuming
-- of coroutines and yielding proper sleeping times to widelands. These
-- functions are more specially tailored to widelands and take a lot of
-- the awkwardness out of using coroutines directly.

-- =======================================================================
--                             PUBLIC FUNCTIONS
-- =======================================================================

-- RST
-- .. function:: run(func[, ...])
--
--    Start to run a function as a coroutine and hand it over to widelands
--    for periodical resuming. All arguments passed to this function are
--    given to the coroutine when it is first run.
--
--    :arg func: Lua function to launch as a coroutine
--
--    :returns: :const:`nil`
function run(func, ...)
   local c = coroutine.create(func)
   local success, sleeptime = coroutine.resume(c, ...)
   if success then
      if coroutine.status(c) ~= "dead" then
         wl.Game():launch_coroutine(c, sleeptime)
      end
   else
      error(sleeptime)
   end
end

-- RST
-- .. function:: sleep(time)
--
--    This must be called inside a coroutine. This will put the coroutine to
--    sleep. Widelands will wake it after the given amount of time.
--
--    :arg time: time to sleep in ms
--    :type time: :class:`integer`
--
--    :returns: :const:`nil`
function sleep(time)
   coroutine.yield(wl.Game().time + time)
end

-- RST
-- .. function:: wake_me(at)
--
--    This must be called inside a coroutine. This will put the coroutine to
--    sleep. Widelands will wake it at the absolute time given. If this time is
--    already in the past (that is at < :func:`wl.Game.time`), the
--    coroutine will be woken at :func:`wl.Game.time` instead.
--
--    :arg at: when to wake this coroutine
--    :type at: :class:`integer`
--
--    :returns: :const:`nil`
function wake_me(at)
   if (at < wl.Game().time) then at = wl.Game().time  end
   coroutine.yield(at)
end
