
function story_messages()
   function _story_box(msg)
      show_story_box(msg.title, msg.msg, msg.loc)
   end

   wake_me(75 * 1000)
   _story_box(story_msg1)
   
   wake_me(160 * 1000)
   _story_box(story_msg2)
   
   wake_me(260 * 1000)
   _story_box(story_msg3)
   
   wake_me(365 * 1000)
   _story_box(story_msg4)
end

run(story_messages)
