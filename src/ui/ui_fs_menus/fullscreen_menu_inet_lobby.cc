/*
 * Copyright (C) 2004 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "constants.h"
#include "fullscreen_menu_inet_lobby.h"
#include "game_server_connection.h"
#include "game_server_proto_packet_hello.h"
#include "game_server_proto_packet_chatmessage.h"
#include "game_server_proto_packet_connect.h"
#include "game_server_proto_packet_getuserinfo.h"
#include "ui_button.h"
#include "ui_editbox.h"
#include "ui_listselect.h"
#include "ui_modal_messagebox.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"
#include "unicode.h"
#include "util.h"

/*
 * Static callback functions for various packets
 */
static void user_entered(std::string name, std::string room, bool enters, void* data ) {
   Fullscreen_Menu_InetLobby* lob = static_cast<Fullscreen_Menu_InetLobby*>(data);
   lob->user_entered(name,room,enters);
}
static void server_message(std::string str, void* data) {
   Fullscreen_Menu_InetLobby* lob = static_cast<Fullscreen_Menu_InetLobby*>(data);
   lob->server_message(str);
}
static void room_info(std::vector< std::string > users, void* data) {
   Fullscreen_Menu_InetLobby* lob = static_cast<Fullscreen_Menu_InetLobby*>(data);
   lob->room_info( users );
}
static void user_info(std::string user, std::string game, std::string room, void* data) {
   Fullscreen_Menu_InetLobby* lob = static_cast<Fullscreen_Menu_InetLobby*>(data);
   lob->user_info( user, game, room );
}
static void chat_message( std::string user, std::string msg, uchar is_action, void* data) {
   Fullscreen_Menu_InetLobby* lob = static_cast<Fullscreen_Menu_InetLobby*>(data);
   lob->chat_message( user, msg, is_action );
}
static void critical_error(std::string str, void* data) {
   Fullscreen_Menu_InetLobby* lob = static_cast<Fullscreen_Menu_InetLobby*>(data);
   lob->critical_error(str);
}
static void disconnect( void* data ) {
   Fullscreen_Menu_InetLobby* lob = static_cast<Fullscreen_Menu_InetLobby*>(data);
   lob->disconnect();
}

/*
 * Init this, send a hello packet
 */
Fullscreen_Menu_InetLobby::Fullscreen_Menu_InetLobby(Game_Server_Connection* gsc)
	:Fullscreen_Menu_Base("singleplmenu.jpg") // change this
{
   m_disconnect_expected = false;

   // Text
	UITextarea* title= new UITextarea(this, MENU_XRES/2, 10, _("Lobby"), Align_HCenter);
	title->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

	// Chat area
   m_chatarea = new UIMultiline_Textarea(this, 5, 40, (int)(get_inner_w()*0.75)-5, get_inner_h()-40-50, 0, Align_Left, 1);
   m_chatarea->set_scrollmode(UIMultiline_Textarea::ScrollLog);

   // Chat editbox
   m_chatbox = new UIEdit_Box(this, 5, get_inner_h()-45, (int)(get_inner_w()*0.75)-5, 25, 0, 1);
   m_chatbox->changed.set(this, &Fullscreen_Menu_InetLobby::changed);

   // User Listing
   m_userlist = new UIListselect(this, (int)(get_inner_w()-get_inner_w()*0.25+5), 40, (int)(get_inner_w()*0.25-10), get_inner_h()-40-50, Align_Left);

   // Buttons
   UIButton* b = new UIButton(this, (int)(get_inner_w()-get_inner_w()*0.25+5), get_inner_h()-45, 50, 25, 0, 0);
   b->set_title(_("Back"));
   b->clickedid.set(this, &Fullscreen_Menu_InetLobby::clicked);

   m_gsc = gsc;
   gsc->set_server_message_handler(&::server_message, this);
   gsc->set_user_entered_handler(&::user_entered, this);
   gsc->set_get_room_info_handler(&::room_info, this);
   gsc->set_get_user_info_handler(&::user_info, this);
   gsc->set_chat_message_handler(&::chat_message, this);
   gsc->set_critical_error_handler(&::critical_error, this);
   gsc->set_disconnect_handler(&::disconnect, this);

   // Send welcome packet(s)
   Game_Server_Protocol_Packet_Connect* gp = new Game_Server_Protocol_Packet_Connect();
   m_gsc->send(gp);
   Game_Server_Protocol_Packet_Hello* hello = new Game_Server_Protocol_Packet_Hello(gsc->get_username());
   m_gsc->send(hello);
}

Fullscreen_Menu_InetLobby::~Fullscreen_Menu_InetLobby()
{
}

/*
 * Check if there is network data for us
 */
void Fullscreen_Menu_InetLobby::think( void ) {
   m_gsc->handle_data();
}

/*
 * The editbox has changed, this is to send something over the net
 */
void Fullscreen_Menu_InetLobby::changed( void ) {
   std::string text = m_chatbox->get_text();
   m_chatbox->set_text("");

   // TODO: check here if this is a Server message
   Game_Server_Protocol_Packet_ChatMessage* cm = new Game_Server_Protocol_Packet_ChatMessage(0,text);
   m_gsc->send(cm);
}

/*
 * A button has been clicked
 */
void Fullscreen_Menu_InetLobby::clicked( int n ) {
   // TODO: shut down connection gracefully
   end_modal(n);
}

/*
 * a server message has reached us. This is text for the user
 * but not written by any other user, but send by the
 * server (as response to a packet mostly) 
 */
void Fullscreen_Menu_InetLobby::server_message(std::string str) {
   std::string buf = str; 
   
   m_chatarea->set_text( (m_chatarea->get_text() + buf).c_str() );
}

/*
 * A room info request has returned
 */
void Fullscreen_Menu_InetLobby::room_info( std::vector<std::string > users ) {
   /* We clear the list, and refill it */
   m_userlist->clear();

   for(uint i = 0; i < users.size(); i++ ) {
      if(users[i] == m_gsc->get_username()) continue;
      std::string name = users[i];
      m_userlist->add_entry(name.c_str(), 0);
      // Get User Informations
      Game_Server_Protocol_Packet_GetUserInfo *gui = new Game_Server_Protocol_Packet_GetUserInfo(users[i]);
      m_gsc->send(gui);
   }
   m_userlist->sort();
}

/*
 * A user info request has returned
 */
void Fullscreen_Menu_InetLobby::user_info( std::string user, std::string game, std::string room ) {
   char buffer[1024];
  
   snprintf(buffer, 1024, _("User %s is using \"%s\" in room \"%s\"\n"), user.c_str(), game.c_str(), room.c_str()); 
   server_message( buffer );
}

/*
 * A chat message has arrived
 */
void Fullscreen_Menu_InetLobby::chat_message( std::string user, std::string msg, bool is_action) {
   if(is_action) {
      char buffer[msg.size()+user.size()+100];

      snprintf(buffer, msg.size()+user.size()+100, "** %s %s\n", user.c_str(), msg.c_str());
      server_message( buffer );
      return;
   } 
      
   char buffer[msg.size()+user.size()+100];

   snprintf(buffer, msg.size()+user.size()+100, "%s: %s\n", user.c_str(), msg.c_str());
   std::string text = m_chatbox->get_text();
   m_chatarea->set_text( (m_chatarea->get_text() + std::string(buffer)).c_str() );
}


/* 
 * A User entered the room 
 */
void Fullscreen_Menu_InetLobby::user_entered(std::string gname, std::string groom, bool enters ) {
   std::string name = gname; 
  
   if( enters && groom == m_gsc->get_room() ) 
      m_userlist->add_entry(name.c_str(), 0);
   if( !enters && groom == m_gsc->get_room() )
      m_userlist->remove_entry(name.c_str());
   
   m_userlist->sort();

   std::string str = enters ? _("User %s has entered the room %s !\n") : _("User %s has left the room %s !\n");
   server_message( str );

   // Get User Informations
   Game_Server_Protocol_Packet_GetUserInfo *gui = new Game_Server_Protocol_Packet_GetUserInfo(gname);
   m_gsc->send(gui);
}

/*
 * A critical connection error has occured. It
 * is likely that the connection is terminated, but
 * we only have to care for showing the user the
 * message
 */
void Fullscreen_Menu_InetLobby::critical_error(std::string str) {
   UIModal_Message_Box* mmb = new UIModal_Message_Box(this, _("Critical Connection Error!"), str, UIModal_Message_Box::OK);
   mmb->run();
   m_disconnect_expected = true;
   delete mmb;
}

/*
 * A disconnect occured
 */
void Fullscreen_Menu_InetLobby::disconnect( void ) {
   if(!m_disconnect_expected) {
      UIModal_Message_Box* mmb = new UIModal_Message_Box(this, _("Critical Connection Error!"), _("Server disconnected unexpectedly!"), UIModal_Message_Box::OK);
      mmb->run();
      delete mmb;
   }
   end_modal(0);
   return;
}
   
