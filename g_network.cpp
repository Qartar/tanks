/*
===============================================================================

Name	:	g_network.cpp

Purpose	:	NETORKING!!!

Comments:	this code is messy sloppy and bad, it was hacked together from
			the networking classes that i made for another app, the code
			here is poorly written and hardly intelligble, especially considering
			the lack of comments

			networking was mostly an afterthought for this program, you might
			see that there really isn't any sort of server-client architecture.
			basically clients are their own mini-server, where the real server
			gives it UNCOMPRESSED information EVERY FRAME for EVERY OBJECT

===============================================================================
*/

#include "local.h"
#include "cm_variable.h"

extern cvar_t	*g_upgrade_frac;
extern cvar_t	*g_upgrade_penalty;
extern cvar_t	*g_upgrade_min;

#define	PROTOCOL_VERSION	2

/*
===========================================================

Name	:	m_StartServer	m_StopServer

===========================================================
*/

void cGame::m_StartServer ()
{
	int	nWidth = g_Application->get_glWnd()->get_WndParams().nSize[0];
	int	nHeight = g_Application->get_glWnd()->get_WndParams().nSize[1];

	nWidth -= SPAWN_BUFFER*2;
	nHeight -= SPAWN_BUFFER*2;

	m_StopClient( );

	m_bMultiplayer = true;
	m_bMultiserver = true;
	m_bMultiactive = true;

	Reset( );
//	m_World.Reset( );

	// init local player

	m_InitPlayers( );

	if ( !m_bDedicated )
	{
		svs.clients[0].active = true;
		svs.clients[0].local = true;

		m_Players[0].vPos = vec2(nWidth*frand()+SPAWN_BUFFER,nHeight*frand()+SPAWN_BUFFER);
		m_Players[0].flAngle = frand()*360;
		m_Players[0].flTAngle = m_Players[0].flAngle;

		m_Players[0].vVel = vec2(0,0);
		m_Players[0].flAVel = 0;
		m_Players[0].flTVel = 0;

		m_Players[0].flDamage = 0.0f;

		m_nScore[0] = 0;

		strncpy( svs.clients[0].name, cls.name, SHORT_STRING );

		m_World.AddObject( &m_Players[0] );
	}
	else
	{
		svs.clients[0].active = false;
		svs.clients[0].local = false;
	}

	m_bGameActive = true;
	m_bMenuActive = false;

	svs.active = true;

	m_netchan.Setup( NS_SERVER, m_netfrom );	// remote doesn't matter
}

void cGame::m_StopServer ()
{
	int				i;
	client_t		*cl;

	if ( !m_bMultiserver )
		return;

	svs.active = false;

	m_bMultiplayer = false;
	m_bMultiserver = false;
	m_bMultiactive = false;

	for ( i=0,cl=svs.clients ; i<MAX_PLAYERS ; i++,cl++ )
	{
		if ( cl->local )
			continue;
		if ( !cl->active )
			continue;

		m_ClientDisconnect( i );

		cl->netchan.message.WriteByte( svc_disconnect );
		cl->netchan.Transmit( cl->netchan.message.nCurSize, cl->netchan.messagebuf );
		cl->netchan.message.Clear( );
	}

	m_World.Reset( );

	m_bGameActive = false;
}

void cGame::m_StopClient ()
{
	if ( m_bMultiplayer && !m_bMultiserver && m_bMultiactive )
	{
		m_netchan.message.WriteByte( clc_disconnect );
		m_netchan.Transmit( m_netchan.message.nCurSize, m_netchan.messagebuf );
		m_netchan.message.Clear( );
	}

	for ( int i=0 ; i<MAX_PLAYERS ; i++ )
	{
		if ( m_Players[i].channels[0] )
		{
			m_Players[i].channels[0]->stopSound( );
			m_Players[i].channels[1]->stopSound( );
			m_Players[i].channels[2]->stopSound( );
		}
	}

	m_bMultiplayer = false;
	m_bMultiactive = false;

	m_bGameActive = false;
	m_bMenuActive = true;
}

/*
===========================================================

Name	:	m_GetPackets	m_GetFrame	m_WriteFrame	m_SendPackets

===========================================================
*/

void cGame::m_GetPackets ()
{
	netsock_t	socket;
	client_t	*cl;
	int			i;

	float		time = g_Application->get_time( );

	if ( m_bMultiserver )
		socket = NS_SERVER;
	else
		socket = NS_CLIENT;

	while ( pNet->Get( socket, &m_netfrom, &m_netmsg ) )
	{
		if ( *(int *)m_netmsg.pData == -1 )
		{
			m_Connectionless( socket );
			continue;
		}

		if ( socket == NS_SERVER )
		{
			int		netport;

			m_netmsg.Begin( );
			m_netmsg.ReadLong( );	// trash
			netport = m_netmsg.ReadShort( );

			for ( i=0,cl=svs.clients ; i<MAX_PLAYERS ; i++,cl++ )
			{
				if ( cl->local )
					continue;
				if ( !cl->active )
					continue;
				if ( cl->netchan.address != m_netfrom )
					continue;
				if ( cl->netchan.netport != netport )
					continue;

				// found him
				m_netclient = i;
				break;
			}

			if ( i == MAX_PLAYERS )
				continue;	// bad client

			cl->netchan.Process( &m_netmsg );
		}
		else
		{
			if ( m_netfrom != m_netserver )
				break;	// not from our server

			m_netchan.Process( &m_netmsg );
		}

		m_Packet( socket );
	}

	//
	// check for timeouts
	//

	if ( m_bMultiserver )
	{
		for ( i=0,cl=svs.clients ; i<MAX_PLAYERS ; i++,cl++ )
		{
			if ( cl->local )
				continue;

			if ( !cl->active )
				continue;

			if ( cl->netchan.last_received + 10000 < time )
			{
				cl->netchan.message.WriteByte( svc_disconnect );
				cl->netchan.Transmit( cl->netchan.message.nCurSize, cl->netchan.messagebuf );

				m_WriteMessage( va("%s timed out.", cl->name ) );
				m_ClientDisconnect( i );
			}
		}
	}
	else if ( m_bMultiactive )
	{
		if ( m_netchan.last_received + 10000 < time )
		{
			m_WriteMessage( "Server timed out." );
			m_StopClient( );
		}
	}
}

void cGame::m_Connectionless (netsock_t socket)
{
	static char	*szCmd;	

	m_netmsg.Begin( );
	m_netmsg.ReadLong( );

	szCmd = m_netstring = m_netmsg.ReadString( );

	if ( socket == NS_SERVER )
	{
		if ( (strstr( szCmd, "info" ) ) )
			m_InfoSend( );
		else if ( (strstr( szCmd, "connect" ) ) )
			m_ClientConnect( );
	}
	else if ( socket == NS_CLIENT )
	{
		if ( (strstr( szCmd, "info" ) ) )
			m_InfoGet( );
		else if ( (strstr( szCmd, "connect" ) ) )
			m_ConnectAck( );
		else if ( (strstr( szCmd, "fail" ) ) )
			m_ReadFail( );
	}
}

void cGame::m_Packet (netsock_t socket)
{
	int		net_cmd;
	char	*string;
	int		net_read;

	while ( true )
	{
		if ( m_netmsg.nReadCount > m_netmsg.nCurSize )
			break;

		net_cmd = m_netmsg.ReadByte( );

		if ( net_cmd == -1 )
			return;

		if ( socket == NS_SERVER )
		{
			switch ( net_cmd )
			{
			case clc_command:
				m_ClientCommand( );
				break;

			case clc_disconnect:
				m_WriteMessage( va("%s disconnected.", svs.clients[m_netclient].name ) );
				m_ClientDisconnect( m_netclient );
				break;

			case clc_say:
				string = m_netmsg.ReadString( );
				m_WriteMessage( va( "\\c%02x%02x%02x%s\\cx: %s",
					(int )(m_Players[m_netclient].vColor.r * 255),
					(int )(m_Players[m_netclient].vColor.g * 255),
					(int )(m_Players[m_netclient].vColor.b * 255),
					svs.clients[m_netclient].name, string ) ); 
				break;

			case clc_upgrade:
				net_read = m_netmsg.ReadByte( );
				m_ReadUpgrade( net_read );
				break;

			case svc_info:
				m_ReadInfo( );
				break;

			default:
				return;
			}
		}
		else if ( socket == NS_CLIENT )
		{
			switch ( net_cmd )
			{
			case svc_disconnect:
				m_bMultiactive = false;
				m_WriteMessage( "Disconnected from server." );
				m_StopClient( );
				break;
			case svc_message:
				string = m_netmsg.ReadString( );
				m_WriteMessage( string );
				break;
			case svc_score:
				net_read = m_netmsg.ReadByte( );
				m_nScore[net_read] = m_netmsg.ReadByte( );
				break;
			case svc_info:
				m_ReadInfo( );
				break;
			case svc_frame:
				m_GetFrame( );
				break;
			case svc_effect:
				m_ReadEffect( );
				break;
			case svc_sound:
				m_ReadSound( );
				break;
			default:
				return;
			}
		}
	}
}

void cGame::m_Broadcast (int len, byte *data)
{
	int				i;
	client_t		*cl;

	for ( i=0,cl=svs.clients ; i<MAX_PLAYERS ; i++,cl++ )
	{
		if ( cl->local )
			continue;
		if ( !cl->active )
			continue;

		cl->netchan.message.Write( data, len );
	}
}

void cGame::m_Broadcast_Print (char *message)
{
	netmsg_t	netmsg;
	byte		netmsgbuf[MAX_MSGLEN];

	netmsg.Init( netmsgbuf, MAX_MSGLEN );
		
	netmsg.WriteByte( svc_message );
	netmsg.WriteString( message );

	m_Broadcast( netmsg.nCurSize, netmsgbuf );
}

#define CLAMP_TIME	10.0f

void cGame::m_GetFrame ()
{
	int				i;
	int				readbyte;
	int				framenum;

	m_World.Reset( );

	framenum = m_netmsg.ReadLong( );

	if ( framenum < cls.last_frame )
		return;
	
	cls.last_frame = framenum;
	m_nFramenum = framenum;

	//
	// allow some leeway for arriving packets, if they exceed it
	// clamp the time so that the velocity lerping doesn't goof up
	//

	// low clamp
	if ( ((m_flTime + CLAMP_TIME) < (float)m_nFramenum * FRAMEMSEC) )
		m_flTime = (float)m_nFramenum * FRAMEMSEC;

	// high clamp
	if 	( ((m_flTime - CLAMP_TIME) > (float)m_nFramenum * FRAMEMSEC) )
		m_flTime = (float)m_nFramenum * FRAMEMSEC;

	i = 0;
	while ( true )
	{
		readbyte = m_netmsg.ReadByte( );
		if ( !readbyte )
			break;

		i = m_netmsg.ReadByte( );

		m_Players[i].vPos		= m_netmsg.ReadVector( );
		m_Players[i].vVel		= m_netmsg.ReadVector( );
		m_Players[i].flAngle	= m_netmsg.ReadFloat( );
		m_Players[i].flAVel		= m_netmsg.ReadFloat( );
		m_Players[i].flTAngle	= m_netmsg.ReadFloat( );
		m_Players[i].flTVel		= m_netmsg.ReadFloat( );

		m_Players[i].flDamage	= m_netmsg.ReadFloat( );
		m_Players[i].flLastFire	= m_netmsg.ReadFloat( );

		m_World.AddObject( &m_Players[i] );

		// this is normally run on cTank::Think but is this 
		// not called on clients and must be called here
		m_Players[i].UpdateSound( );


		readbyte = m_netmsg.ReadByte( );

		if ( readbyte )
		{
			m_Players[i].m_Bullet.vPos = m_netmsg.ReadVector( );
			m_Players[i].m_Bullet.vVel = m_netmsg.ReadVector( );

			m_World.AddObject( &m_Players[i].m_Bullet );
		}
	}

	return;
}

void cGame::m_WriteFrame ()
{
	netmsg_t	message;
	static byte	messagebuf[MAX_MSGLEN];

	int				i;
	client_t		*cl;

	// HACK: update local players color here
	if ( svs.clients[0].local )
		m_Players[0].vColor = cls.color;

	message.Init( messagebuf, MAX_MSGLEN );
	message.Clear( );

	message.WriteByte( svc_frame );
	message.WriteLong( m_nFramenum );
	for ( i=0,cl=svs.clients ; i<MAX_PLAYERS ; i++,cl++ )
	{
		if ( !cl->active )
			continue;

		message.WriteByte( 1 );
		message.WriteByte( i );

		message.WriteVector( m_Players[i].vPos );
		message.WriteVector( m_Players[i].vVel );
		message.WriteFloat( m_Players[i].flAngle );
		message.WriteFloat( m_Players[i].flAVel );
		message.WriteFloat( m_Players[i].flTAngle );
		message.WriteFloat( m_Players[i].flTVel );

		message.WriteFloat( m_Players[i].flDamage );
		message.WriteFloat( m_Players[i].flLastFire );

		if ( m_Players[i].m_Bullet.bInGame )
		{
			message.WriteByte( 1 );

			message.WriteVector( m_Players[i].m_Bullet.vPos );
			message.WriteVector( m_Players[i].m_Bullet.vVel );
		}
		else
			message.WriteByte( 0 );
	}

	message.WriteByte( 0 );

	m_Broadcast( message.nCurSize, messagebuf );

	return;
}

void cGame::m_SendPackets ()
{
	int			i;
	client_t	*cl;

	if ( !m_bMultiactive )
		return;

	if ( m_bMultiserver )
		goto server;

	m_clientsend( );	// write move commands

	if ( m_netchan.message.nCurSize )
	{
		m_netchan.Transmit( m_netchan.message.nCurSize, m_netchan.messagebuf );
		m_netchan.message.Clear( );
	}

	return;

server:
	for ( i=0,cl=svs.clients ; i<MAX_PLAYERS ; i++,cl++ )
	{
		if ( cl->local )
			continue;	// local

		if ( !cl->active )
			continue;

		if ( !cl->netchan.message.nCurSize )
			continue;

		cl->netchan.Transmit( cl->netchan.message.nCurSize, cl->netchan.messagebuf );
		cl->netchan.message.Clear( );
	}
}

/*
===========================================================

Name	:	m_ClientConnect	m_ClientDisconnect	m_ClientPacket

===========================================================
*/

void cGame::m_ConnectToServer (int index)
{
	// ask server for a connection

	m_StopClient( );
	m_StopServer( );

	if ( index > 0 )
		m_netserver = cls.servers[index].address;

	m_netserver.type = NA_IP;
	if ( !m_netserver.port )
		m_netserver.port = BIG_SHORT( PORT_SERVER );

	pNet->Print( NS_CLIENT, m_netserver, va( "connect %i %s %i", PROTOCOL_VERSION, cls.name, m_netchan.netport ) );
}

void cGame::m_ConnectAck ()
{
	int		i;

	char	tempbuf[32];

	// server has ack'd our connect

	sscanf( m_netstring, "%s %i", tempbuf, &cls.number );

	m_netchan.Setup( NS_CLIENT, m_netserver );

	m_flTime = 0.0f;
	cls.last_frame = 0;

	m_bMultiplayer = true;		// wtf are all these bools?
	m_bMultiactive = true;		// i dont even remember

	m_bMenuActive = false;		// so you're totally screwed
	m_bGameActive = true;		// if you ever want to know

	for ( i=0 ; i<MAX_PLAYERS ; i++ )
	{
		if ( m_Players[i].channels[0] )
		{
			m_Players[i].channels[0]->stopSound( );
			m_Players[i].channels[1]->stopSound( );
			m_Players[i].channels[2]->stopSound( );
		}
	}

	m_Players[cls.number].vColor.r = cls.color.r;
	m_Players[cls.number].vColor.g = cls.color.g;
	m_Players[cls.number].vColor.b = cls.color.b;

	gameClients[cls.number].upgrades = 0;
	gameClients[cls.number].damage_mod = 1.0f;
	gameClients[cls.number].armor_mod = 1.0f;
	gameClients[cls.number].refire_mod = 1.0f;
	gameClients[cls.number].speed_mod = 1.0f;

	svs.clients[cls.number].active = true;
	strcpy( svs.clients[cls.number].name, cls.name );

	m_WriteInfo( cls.number, &m_netchan.message );
}

void cGame::m_ClientConnect ()
{
	// client has asked for connection

	int			i;
	client_t	*cl;
	int			netport, version;
	char		tempbuf[SHORT_STRING];
	char		clname[SHORT_STRING];

	netmsg_t	message;
	byte		messagebuf[MAX_MSGLEN];

	message.Init( messagebuf, MAX_MSGLEN );

	int	nWidth = 640;
	int	nHeight = 480;

	nWidth -= SPAWN_BUFFER*2;
	nHeight -= SPAWN_BUFFER*2;

	if ( !m_bMultiserver )
		return;

	sscanf( m_netstring, "%s %i %s %i", tempbuf, &version, clname, &netport );

	if ( version != PROTOCOL_VERSION )
	{
		pNet->Print( NS_SERVER, m_netfrom, va("fail \"Bad protocol version: %i\"", version )  );
		return;
	}

	cl = NULL;
	for ( i=0 ; i<svs.max_clients ; i++ )
	{
		if ( !svs.clients[i].active )
		{
			cl = &svs.clients[i];
			break;
		}
		if ( svs.clients[i].netchan.address == m_netfrom )
		{
			// this client is already connected
			if ( svs.clients[i].netchan.netport == netport )
				return;
		}
	}

	if ( !cl )	// couldn't find client slot
	{
		pNet->Print( NS_SERVER, m_netfrom, "fail \"Server is full\"" );
		return;
	}

	// add new client

	cl->active = true;
	cl->local = false;
	cl->netchan.Setup( NS_SERVER, m_netfrom, netport );

	strncpy( cl->name, clname, SHORT_STRING );

	pNet->Print( NS_SERVER, cl->netchan.address, va( "connect %i", i ) );

	// init their tank

	m_Players[i].vPos = vec2(nWidth*frand()+SPAWN_BUFFER,nHeight*frand()+SPAWN_BUFFER);
	m_Players[i].flAngle = frand()*360;
	m_Players[i].flTAngle = m_Players[i].flAngle;

	m_Players[i].vVel = vec2(0,0);
	m_Players[i].flAVel = 0;
	m_Players[i].flTVel = 0;

	m_Players[i].flDamage = 0.0f;

	gameClients[i].armor_mod = 1.0f;
	gameClients[i].damage_mod = 1.0f;
	gameClients[i].refire_mod = 1.0f;
	gameClients[i].speed_mod = 1.0f;
	gameClients[i].upgrades = 0;

	m_nScore[i] = 0;

	// add into world

	m_World.AddObject( &m_Players[i] );

	m_WriteMessage( va( "%s connected.", cl->name ) );

	for ( i=0 ; i<svs.max_clients ; i++ )
	{
		if ( cl == &svs.clients[i] )
			continue;

		m_WriteInfo( i, &cl->netchan.message );
	}

	return;
}

void cGame::m_ReadFail ()
{
	textutils_c	text;

	text.parse( m_netstring );

	m_WriteMessage( va( "Failed to connect: %s", text.argv(1) ) );
}

void cGame::m_WriteInfo (int client, netmsg_t *message)
{
	message->WriteByte( svc_info );
	message->WriteByte( client );
	message->WriteByte( svs.clients[client].active );
	message->WriteString( svs.clients[client].name );

	//	write extra shit

	message->WriteByte( gameClients[client].upgrades );
	message->WriteByte( gameClients[client].armor_mod * 10 );
	message->WriteByte( gameClients[client].damage_mod * 10 );
	message->WriteByte( gameClients[client].refire_mod * 10 );
	message->WriteByte( gameClients[client].speed_mod * 10 );

	message->WriteByte( m_Players[client].vColor.r * 255 );
	message->WriteByte( m_Players[client].vColor.g * 255 );
	message->WriteByte( m_Players[client].vColor.b * 255 );

	// also write score

	message->WriteByte( svc_score );
	message->WriteByte( client );
	message->WriteByte( m_nScore[client] );
}

void cGame::m_ReadInfo ()
{
	int		client;
	int		active;
	char	*string;

	client = m_netmsg.ReadByte( );
	active = m_netmsg.ReadByte( );

	svs.clients[client].active = ( active == 1 );
	
	string = m_netmsg.ReadString( );
	strncpy( svs.clients[client].name, string, SHORT_STRING );

	gameClients[client].upgrades = m_netmsg.ReadByte( );
	gameClients[client].armor_mod = m_netmsg.ReadByte( ) / 10.0f;
	gameClients[client].damage_mod = m_netmsg.ReadByte( ) / 10.0f;
	gameClients[client].refire_mod = m_netmsg.ReadByte( ) / 10.0f;
	gameClients[client].speed_mod = m_netmsg.ReadByte( ) / 10.0f;

	m_Players[client].vColor.r = m_netmsg.ReadByte( ) / 255.0f;
	m_Players[client].vColor.g = m_netmsg.ReadByte( ) / 255.0f;
	m_Players[client].vColor.b = m_netmsg.ReadByte( ) / 255.0f;

	if ( m_bMultiserver || m_bDedicated )
	{
		netmsg_t	netmsg;
		byte		msgbuf[MAX_MSGLEN];

		netmsg.Init( msgbuf, MAX_MSGLEN );
		m_WriteInfo( client, &netmsg );
		m_Broadcast( netmsg.nCurSize, netmsg.pData );
	}
}

void cGame::m_ClientDisconnect (int nClient)
{
	netmsg_t	message;
	byte		messagebuf[MAX_MSGLEN];

	message.Init( messagebuf, MAX_MSGLEN );

	if ( !svs.clients[nClient].active )
		return;

	m_World.DelObject( &m_Players[nClient] );
	svs.clients[nClient].active = false;

	m_WriteInfo( nClient, &message );
	m_Broadcast( message.nCurSize, messagebuf );
}

void cGame::m_ClientCommand ()
{
	cTank	*pTank;
	int		bits;

	bits = m_netmsg.ReadByte( );
	pTank = &m_Players[m_netclient];

	memset( pTank->m_Keys, 0, sizeof(pTank->m_Keys) );

	if ( bits & BIT(KEY_FORWARD) )
		pTank->m_Keys[KEY_FORWARD] = true;
	if ( bits & BIT(KEY_BACK) )
		pTank->m_Keys[KEY_BACK] = true;

	if ( bits & BIT(KEY_LEFT) )
		pTank->m_Keys[KEY_LEFT] = true;
	if ( bits & BIT(KEY_RIGHT) )
		pTank->m_Keys[KEY_RIGHT] = true;

	if ( bits & BIT(KEY_TLEFT) )
		pTank->m_Keys[KEY_TLEFT] = true;
	if ( bits & BIT(KEY_TRIGHT) )
		pTank->m_Keys[KEY_TRIGHT] = true;
	if ( bits & BIT(KEY_FIRE) )
		pTank->m_Keys[KEY_FIRE] = true;

//	m_Players[m_netclient].vColor.r = (float)m_netmsg.ReadByte( ) / 255.0f;
//	m_Players[m_netclient].vColor.g = (float)m_netmsg.ReadByte( ) / 255.0f;
//	m_Players[m_netclient].vColor.b = (float)m_netmsg.ReadByte( ) / 255.0f;
}

void cGame::m_clientsend ()
{
	int		bits = 0;

	if ( m_clientkeys[KEY_FORWARD] == true )
		bits |= BIT(KEY_FORWARD);
	if ( m_clientkeys[KEY_BACK] == true )
		bits |= BIT(KEY_BACK);

	if ( m_clientkeys[KEY_LEFT] == true )
		bits |= BIT(KEY_LEFT);
	if ( m_clientkeys[KEY_RIGHT] == true )
		bits |= BIT(KEY_RIGHT);

	if ( m_clientkeys[KEY_TLEFT] == true )
		bits |= BIT(KEY_TLEFT);
	if ( m_clientkeys[KEY_TRIGHT] == true )
		bits |= BIT(KEY_TRIGHT);
	if ( m_clientkeys[KEY_FIRE] == true )
		bits |= BIT(KEY_FIRE);

	m_netchan.message.WriteByte( clc_command );
	m_netchan.message.WriteByte( bits );
	// write color
//	m_netchan.message.WriteByte( cls.color.r * 255.0f );
//	m_netchan.message.WriteByte( cls.color.g * 255.0f );
//	m_netchan.message.WriteByte( cls.color.b * 255.0f );
}

void cGame::m_WriteSound (int nSound)
{
	netmsg_t	netmsg;
	static byte	netmsgbuf[MAX_MSGLEN];

	if ( !m_bMultiserver )
		return;

	netmsg.Init( netmsgbuf, MAX_MSGLEN );
	netmsg.Clear( );

	netmsg.WriteByte( svc_sound );
	netmsg.WriteLong( nSound );

	m_Broadcast( netmsg.nCurSize, netmsgbuf );
}

void cGame::m_ReadSound ()
{
	int	nSound;

	nSound = m_netmsg.ReadLong( );

	pSound->playSound( nSound, vec3(0,0,0), 1.0f, 0.0f );
}

void cGame::m_WriteEffect (int type, vec2 pos, vec2 vel, int count)
{
	netmsg_t	netmsg;
	static byte	netmsgbuf[MAX_MSGLEN];

	if ( !m_bMultiserver )
		return;

	netmsg.Init( netmsgbuf, MAX_MSGLEN );
	netmsg.Clear( );

	netmsg.WriteByte( svc_effect );
	netmsg.WriteByte( type );
	netmsg.WriteVector( pos );
	if ( type == effect_smoke )
	{
		netmsg.WriteVector( vel );
		netmsg.WriteShort( count );
	}

	m_Broadcast( netmsg.nCurSize, netmsgbuf );
}

void cGame::m_ReadEffect ()
{
	int		type;
	vec2	pos;
	vec2	vel;
	int		count;

	type = m_netmsg.ReadByte( );
	pos = m_netmsg.ReadVector( );
	if ( type == effect_smoke )
	{
		vel = m_netmsg.ReadVector( );
		count = m_netmsg.ReadShort( );
		m_World.AddSmokeEffect( pos, vel, count );
	}
	else
		m_World.AddEffect( pos, (effects_t )type );
}

/*
===========================================================

Name	:	m_InfoAsk	m_InfoSend	m_InfoGet

===========================================================
*/

void cGame::m_InfoAsk ()
{
	netadr_t	addr;

	for ( int i=0 ; i<MAX_SERVERS ; i++ )
	{
		cls.servers[i].name[0] = 0;
		cls.servers[i].active = false;
	}

	cls.ping_time = m_flTime;

	m_StopClient( );
	m_StopServer( );

	m_bHaveServer = false;

	pNet->StringToNet( "oedhead.no-ip.org", &addr );
	addr.type = NA_IP;
	addr.port = BIG_SHORT( PORT_SERVER );
	pNet->Print( NS_CLIENT, addr, "info" );

	addr.type = NA_BROADCAST;
	addr.port = BIG_SHORT( PORT_SERVER );

	pNet->Print( NS_CLIENT, addr, "info" );
}

void cGame::m_InfoSend ()
{
	int		i;

	if ( !svs.active )
		return;

	// check for an empty slot
	for ( i=0 ; i<MAX_PLAYERS ; i++ )
		if ( !svs.clients[i].active )
			break;

	// full, shhhhh
	if ( i == MAX_PLAYERS )
		return;

	pNet->Print( NS_SERVER, m_netfrom, va( "info %s", svs.name) );
}

void cGame::m_InfoGet ()
{
	int			i;

	for ( i=0 ; i<MAX_SERVERS ; i++ )
	{
		// already exists and active, abort
		if ( cls.servers[i].address == m_netfrom && cls.servers[i].active )
			return;

		// slot taken, try next
		if ( cls.servers[i].active )
			continue;

		cls.servers[i].address = m_netfrom;
		cls.servers[i].ping = m_flTime - cls.ping_time;

		strcpy( cls.servers[i].name, m_netstring + 5 );

		cls.servers[i].active = true;

		// old server code
		if ( !m_bHaveServer )
		{
			m_netserver = m_netfrom;
			m_bHaveServer = true;
		}

		return;
	}
}

char	*sz_upgrades[] = {
	"damage",
	"armor",
	"gunnery",
	"speed" };

void cGame::m_ReadUpgrade (int index)
{
	if ( gameClients[m_netclient].upgrades )
	{
		netmsg_t	netmsg;
		byte		msgbuf[MAX_MSGLEN];

		gameClients[m_netclient].upgrades--;
		if ( index == 0 )
		{
			gameClients[m_netclient].damage_mod += UPGRADE_FRAC;
			gameClients[m_netclient].refire_mod -= UPGRADE_PENALTY;
			if ( gameClients[m_netclient].refire_mod < UPGRADE_MIN )
				gameClients[m_netclient].refire_mod = UPGRADE_MIN;
		}
		else if ( index == 1 )
		{
			gameClients[m_netclient].armor_mod += UPGRADE_FRAC;
			gameClients[m_netclient].speed_mod -= UPGRADE_PENALTY;
			if ( gameClients[m_netclient].speed_mod < UPGRADE_MIN )
				gameClients[m_netclient].speed_mod = UPGRADE_MIN;
		}
		else if ( index == 2 )
		{
			gameClients[m_netclient].refire_mod += UPGRADE_FRAC;
			gameClients[m_netclient].damage_mod -= UPGRADE_PENALTY;
			if ( gameClients[m_netclient].damage_mod < UPGRADE_MIN )
				gameClients[m_netclient].damage_mod = UPGRADE_MIN;
		}
		else if ( index == 3 )
		{
			gameClients[m_netclient].speed_mod += UPGRADE_FRAC;
			gameClients[m_netclient].armor_mod -= UPGRADE_PENALTY;
			if ( gameClients[m_netclient].armor_mod < UPGRADE_MIN )
				gameClients[m_netclient].armor_mod = UPGRADE_MIN;
		}

		m_WriteMessage( va( "\\c%02x%02x%02x%s\\cx has upgraded their %s!",
			(int )(m_Players[m_netclient].vColor.r * 255),
			(int )(m_Players[m_netclient].vColor.g * 255),
			(int )(m_Players[m_netclient].vColor.b * 255),
			svs.clients[m_netclient].name, sz_upgrades[index] ) ); 

		netmsg.Init( msgbuf, MAX_MSGLEN );
		m_WriteInfo( m_netclient, &netmsg );
		m_Broadcast( netmsg.nCurSize, netmsg.pData );
	}
}

void cGame::m_WriteUpgrade (int upgrade)
{
	if ( m_bMultiserver )
	{
		m_netclient = 0;
		m_ReadUpgrade( upgrade );
		return;
	}

	m_netchan.message.WriteByte( clc_upgrade );
	m_netchan.message.WriteByte( upgrade );
}