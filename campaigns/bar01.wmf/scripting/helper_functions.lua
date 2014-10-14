-- =================
-- Helper functions
-- =================

function send_msg(t)
   t.h = 400
   -- TODO: this jumps. scrolling would be better
   plr:message_box(t.title, t.body, t)
end

function add_obj(t)
   return plr:add_objective(t.name, t.title, t.body)
end
