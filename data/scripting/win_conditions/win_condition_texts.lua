include "scripting/richtext.lua"

-- RST
-- win_condition_texts.lua
-- ---------------------------
--
-- This script contains tables of localized text that are shared by different
-- win conditions.
--
-- To make these tables available include this file at the beginning
-- of a script via:
--
-- .. code-block:: lua
--
--    include "scripting/win_conditions/win_condition_texts.lua"
--
-- All tables provide two values ``title`` and ``body``. Refer to those values
-- like this:
--
-- .. code-block:: lua
--
--    send_to_inbox(plr, won_game.title, won_game.body, {popup=true})
--

-- RST
-- .. data:: won_game
--
--   :values: * ``title=_("Congratulations!")``
--            * ``body=p(_("You have won this game!"))``
--

won_game = {
  title = _("Congratulations!"),
  body = p(_("You have won this game!"))
}

-- RST
-- .. data:: lost_game
--
--   :values: * ``title=_("You are defeated!")``
--            * ``body=p(_("You lost your last warehouse and are therefore defeated. You may continue as spectator if you want."))``
--

lost_game = {
  title = _("You are defeated!"),
  body = p(_("You lost your last warehouse and are therefore defeated. You may continue as spectator if you want."))
}

-- RST
-- .. data:: won_game_over
--
--   :values: * ``title=_("You won")``
--            * ``body=h2(_("You are the winner!"))``
--

won_game_over = {
  title = _("You won"),
  body = h2(_("You are the winner!"))
}

-- RST
-- .. data:: lost_game_over
--
--   :values: * ``title=_("You lost")``
--            * ``body=h2(_("You’ve lost this game!"))``
--

lost_game_over = {
  title = _("You lost"),
  body = h2(_("You’ve lost this game!"))
}

-- RST
-- .. data:: game_status
--
--   :values: * ``title=_("Status")``
--            * ``body=h2(_("Player overview:"))``
--

game_status = {
  title = _("Status"),
  -- TRANSLATORS: This is an overview for all players.
  body = h2(_("Player overview:"))
}
