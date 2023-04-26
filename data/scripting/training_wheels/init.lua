-- RST
--
-- .. _training_wheels:
--
-- Training Wheels
-- ===============
--
-- Training Wheels are Lua coroutines that will teach new players about the game while they play.
-- They will be triggered automatically in any non-scenario single player game,
-- and are independent of the map or tribe being played.
--
-- Triggering a Training Weel
-- --------------------------
--
-- The existing Training Wheels are listed in ``data/scripting/training_wheels/init.lua``.
-- Their names need to be identical with their base file name (without the ``.lua`` file extension).
-- For blocking the execution of a Training Wheel until the player has learned a prerequisite
-- Training Wheel, list them in the Training Wheel's Lua table in the format
-- ``example = { descname = "Example", dependencies = "prerequisite1", "prerequisite1" } },``.
-- The Training Wheel will trigger if any of its prerequisites has been completed.
--
-- All Training Wheel scripts must include ``scripting/training_wheels/utils/lock.lua`` and call the following two functions:
--
-- * ``wait_for_lock(player, training_wheel_name)``:
--   Call this when the script's setup phase is done,
--   so that the messages from multiple training wheels won't interfere with each other.
-- * ``player:mark_training_wheel_as_solved(training_wheel_name)``:
--   Call this when the teaching points have been completed.
--   This will prevent the training wheel from being run again when a new game starts,
--   and it will also signal to the other training wheels that they can try to acquire the lock.
--
-- Which Training Wheels have been completed previously by the player is stored in
-- ``.widelands/save/training_wheels.conf``.
--
-- Designing a Training Wheel
-- --------------------------
--
-- * **Scope:** Each Training Wheel should aim at teaching one concept only.
--   This can be broadened a bit, e.g. when placing a building,
--   the player will also need to connect a road to it.
-- * **Flexibility:** Because Training Wheels should be designed to work with any tribe (even future ones),
--   **do not** use any hard-coded building or worker names etc.
--   Hard-coded ware names are OK if they are very basic, e.g. "log".
--   Everything else should be handled by using generalized attributes.
--   Also, test with different starting conditions, maps and savegames.
-- * **Robustness:** Expect the player not to follow the instructions.
--   This should not cause any crashes.
-- * **Conciseness:** Keep the texts as short as possible, users don't like to read walls of text.
--   Conciseness also helps with keeping the workload down for our translators.
--
-- Message Formatting
-- ------------------
--
-- We should always follow the same formatting pattern to make it easier for the player
-- to distinguish information from action items. We also use images to give visual
-- references to the player.
--
-- * **Explanations:** Use ``p`` for core explanations if there is no appropriate visual reference for them.
--   Otherwise, use ``li_image`` or ``li_object`` to illustrate.
-- * **Actions:** When explaining an action that the player needs to take, use
--   ``li_image`` or ``li_object`` to give a visual reference. If no appropriate images
--   are available, use ``li`` to show it as a bullet point.
-- * **Hints:** Hints with further information are shown with ``li_arrow``.
--
-- If you need text that's a bit longer, split it into separate translation units.
-- You can then concatenate them with the ``join_sentences`` function.
--
--
-- Example Training Wheel
-- ----------------------
--
-- A Training Wheel in a file ``data/scripting/training_wheels/example.lua`` could look like this:
--
-- .. code-block:: lua
--
--    include "scripting/coroutine.lua"
--    include "scripting/messages.lua"
--    include "scripting/richtext_scenarios.lua"
--    include "scripting/ui.lua"
--    include "scripting/training_wheels/utils/lock.lua"
--    include "scripting/training_wheels/utils/ui.lua"
--
--    -- This needs to be called outside of the coroutine, otherwise ``__file__`` would be ``nil``.
--    -- Using this function rather than just typing ``"example"`` will save us from typos
--    local training_wheel_name = training_wheel_name_from_filename(__file__)
--
--    run(function()
--       sleep(100)
--
--       local interactive_player_slot = wl.Game().interactive_player
--       local player = wl.Game().players[interactive_player_slot]
--
--       wait_for_lock(player, training_wheel_name)
--
--       -- All set. Define our messages now.
--       push_textdomain("training_wheels")
--
--       local msg_example = {
--          title = _("Example"),
--          position = "topright",
--          body = (
--             p("This is an example with a non-modal story message box, so we can let the player do things while we show this message.") ..
--             li("Dear player, please do something.") ..
--             li_arrow("BTW: This is a teachy Training Wheel")
--          ),
--          h = 380,
--          w = 260,
--          modal = false
--       }
--
--       pop_textdomain()
--
--       -- Point to the starting field and show the message
--       local starting_field = wl.Game().map.player_slots[interactive_player_slot].starting_field
--       starting_field:indicate(true)
--       campaign_message_box(msg_example)
--       scroll_to_field(target_field)
--       -- Check here whether the player completed the task
--       sleep(2000)
--       close_story_messagebox()
--       starting_field:indicate(false)
--
--       -- Teaching is done, so mark it as solved. Note that this matches the base filename.
--       player:mark_training_wheel_as_solved(training_wheel_name)
--    end)
--
-- And the corresponding entry in ``init.lua``:
--
-- .. code-block:: lua
--
--    example = {
--       descname = "Example",
--       dependencies = { "objectives", "logs" }
--    },
--

push_textdomain("training_wheels")

result = {
   welcome = {
      -- TRANSLATORS: The title of an in-game teaching objective
      descname = _("Welcome"),
      dependencies = {}
   },
   objectives = {
      -- TRANSLATORS: The title of an in-game teaching objective
      descname = _("Objectives"),
      dependencies = { "welcome" }
   },
   -- TODO(GunChleoc): teach map movement before we start building
   building_spaces = {
      -- TRANSLATORS: The title of an in-game teaching objective
      descname = _("Building Spaces"),
      dependencies = { "objectives" }
   },
   logs = {
      -- TRANSLATORS: The title of an in-game teaching objective
      descname = _("Logs & Roads"),
      dependencies = { "objectives" }
   },
   rocks = {
      -- TRANSLATORS: The title of an in-game teaching objective
      descname = _("Rocks"),
      dependencies = { "objectives" }
   },
   flags = {
      -- TRANSLATORS: The title of an in-game teaching objective
      descname = _("Flags"),
      dependencies = { "logs", "rocks" }
   },
}

pop_textdomain()

return result
