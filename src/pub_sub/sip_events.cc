//--------------------------------------------------------------------------
// Copyright (C) 2016-2016 Cisco and/or its affiliates. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License Version 2 as published
// by the Free Software Foundation.  You may not use, modify or distribute
// this program under any other version of the GNU General Public License.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//--------------------------------------------------------------------------
// sip_events.cc author Carter Waxman <cwaxman@cisco.com>

#include "sip_events.h"

#include "service_inspectors/sip/sip_common.h"
#include "service_inspectors/sip/sip_dialog.h"

using namespace std;

SipEvent::SipEvent(const Packet* p, const SIPMsg* msg, const SIP_DialogData* dialog)
{
    this->p = p;
    this->msg = msg;
    this->dialog = dialog;

    if( msg->from )
        from = string(msg->from, msg->fromLen);

    if( msg->userName )
        user_name = string(msg->userName, msg->userNameLen);

    if( msg->userAgent )
        user_agent = string(msg->userAgent, msg->userAgentLen);

    if( msg->server )
        server = string(msg->server, msg->serverLen);
}

SipEvent::~SipEvent()
{
    for( auto& session : sessions )
        delete session;
}

bool SipEvent::is_invite() const
{ return msg->methodFlag == SIP_METHOD_INVITE; }

bool SipEvent::is_media_updated() const
{ return msg->mediaUpdated; }

bool SipEvent::has_dialog() const
{ return dialog; }

bool SipEvent::is_dialog_established() const
{ return has_dialog() && dialog->state == SIP_DLG_ESTABLISHED; }

void SipEvent::begin_media_sessions()
{
    if( has_dialog() )
        current_media_session = dialog->mediaSessions;
}

SipEventMediaSession* SipEvent::next_media_session()
{
    if( !current_media_session )
        return nullptr;
    
    auto session = new SipEventMediaSession(current_media_session);
    sessions.push_front(session);

    current_media_session = current_media_session->nextS;

    return session;
}


SipEventMediaSession::~SipEventMediaSession()
{
    for( auto& d : data )
        delete d;
}

uint32_t SipEventMediaSession::get_id() const
{ return session->sessionID; }

void SipEventMediaSession::begin_media_data()
{ current_media_data = session->medias; }

SipEventMediaData* SipEventMediaSession::next_media_data()
{
    if( !current_media_data )
        return nullptr;
    
   auto d = new SipEventMediaData(current_media_data);
   data.push_front(d);
   
   current_media_data = current_media_data->nextM;
   
   return d; 
}


const sfip_t* SipEventMediaData::get_address() const
{ return &data->maddress; }

uint16_t SipEventMediaData::get_port() const
{ return data->mport; }
