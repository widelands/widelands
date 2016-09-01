--  =======================================================
--  ****** Texts that appear in multiple help files *******
--  =======================================================

-- RST
-- global_helptexts.lua
-- --------------------
--
-- This file contains some default building helptexts that can be used when a more
-- specific help text hasn't been defined yet. It also contains some global time naming
-- functions for uniform translations.

-- RST
-- .. function:: no_lore_text_yet()
--
--    Returns a localized string for when no lore helptext has been defined yet.
--    :returns: _"Text needed"
--
function no_lore_text_yet()
   -- TRANSLATORS: Lore helptext for a building - it hasn't been written yet.
   return _"Text needed"
end

-- RST
-- .. function:: no_lore_author_text_yet()
--
--    Returns a localized string for when no lore author helptext has been defined yet.
--    :returns: _"Source needed"
--
function no_lore_author_text_yet()
   -- TRANSLATORS: Lore author (source for a quote) helptext for a building - it hasn't been written yet.
   return _"Source needed"
end

-- RST
-- .. function:: no_purpose_text_yet()
--
--    Returns a localized string for when no purpose helptext has been defined yet.
--    :returns: _"Text needed"
--
function no_purpose_text_yet()
   -- TRANSLATORS: Purpose helptext for a building - it hasn't been written yet.
   return _"Text needed"
end

-- RST
-- .. function:: no_performance_text_yet()
--
--    Returns a localized string for when no performance helptext has been defined yet.
--    :returns: _"Calculation needed"
--
function no_performance_text_yet()
   -- TRANSLATORS: Performance helptext for a building - it hasn't been written yet.
   return _"Calculation needed"
end

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
   return ngettext("%d second", "%d seconds", seconds):bformat(seconds)
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
   return ngettext("%d minute", "%d minutes", minutes):bformat(minutes)
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
   return _("%1% and %2%"):bformat(
      ngettext("%d minute", "%d minutes", minutes):bformat(minutes),
      ngettext("%d second", "%d seconds", seconds):bformat(seconds)
   )
end
