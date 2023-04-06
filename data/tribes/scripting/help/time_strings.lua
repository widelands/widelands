--  =======================================================
--  ****** Texts that appear in multiple help files *******
--  =======================================================

-- RST
-- .. _lua_tribes_time_strings:
--
-- time_strings.lua
-- ----------------
--
-- This file contains some global time naming functions for uniform translations.


-- RST
-- .. function:: format_seconds(seconds)
--
--    :arg seconds: number of seconds
--    :type seconds: An unsigned integer
--
--    Returns a localized string to tell the time in seconds with the proper plural form.
--    :returns: "1 second", or "20 seconds" etc.
--
function format_seconds(seconds)
   -- Push and pop textdomains for usage in campaign scenario helptexts
   push_textdomain("tribes_encyclopedia")
   local result = ngettext("%d second", "%d seconds", seconds):bformat(seconds)
   pop_textdomain()
   return result
end

-- RST
-- .. function:: format_minutes(minutes)
--
--    :arg minutes: number of minutes
--    :type minutes: An unsigned integer
--
--    Returns a localized string to tell the time in minutes with the proper plural form.
--    :returns: "1 minute", or "20 minutes" etc.
--
function format_minutes(minutes)
   push_textdomain("tribes_encyclopedia")
   local result = ngettext("%d minute", "%d minutes", minutes):bformat(minutes)
   pop_textdomain()
   return result
end

-- RST
-- .. function:: format_minutes_seconds(minutes, seconds)
--
--    :arg minutes: number of minutes
--    :type minutes: An unsigned integer
--    :arg seconds: number of seconds
--    :type seconds: An unsigned integer
--
--    Returns a localized string to tell the time in minutes and seconds with the proper plural form.
--    :returns: "1 minute and 20 seconds" etc.
--
function format_minutes_seconds(minutes, seconds)
   push_textdomain("tribes_encyclopedia")
   local result = _("%1% and %2%"):bformat(
      ngettext("%d minute", "%d minutes", minutes):bformat(minutes),
      ngettext("%d second", "%d seconds", seconds):bformat(seconds)
   )
   pop_textdomain()
   return result
end
