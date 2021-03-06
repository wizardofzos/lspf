/*  Compile with ::                                                                    */
/* g++ -shared -fPIC -std=c++11 -Wl,-soname,libPPSP01A.so -o libPPSP01A.so PPSP01A.cpp */

/*
  Copyright (c) 2015 Daniel John Erdos

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/


/* General utilities:                                                       */
/* lspf LOG                                                                 */
/* Application LOG                                                          */
/* PFKEY Settings                                                           */
/* COLOUR Settings                                                          */
/* T0DO list                                                                */
/* SHARED and PROFILE Variable display                                      */
/* LIBDEF status                                                            */
/* Display LIBDEF status and search paths                                   */
/* Display loaded Modules (loaded Dynamic Classes)                          */
/* Display Saved File List                                                  */
/* Simple task list display (from ps aux output)                            */
/* RUN an application (default parameters)                                  */

#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>

#include "../lspf.h"
#include "../utilities.h"
#include "../classes.h"
#include "../pVPOOL.h"
#include "../pWidgets.h"
#include "../pTable.h"
#include "../pPanel.h"
#include "../pApplication.h"

#include "PPSP01A.h"

using namespace std ;
using namespace boost::filesystem ;

#undef  MOD_NAME
#define MOD_NAME PPSP01A

boost::mutex PPSP01A::mtx ;


PPSP01A::PPSP01A()
{
	vdefine( "ZCURINX ZTDTOP ZTDSELS", &zcurinx, &ztdtop, &ztdsels ) ;
}


void PPSP01A::application()
{
	llog( "I", "Application PPSP01A starting" << endl ) ;

	set_appdesc( "General utilities to display logs, PF Key settings, variables, etc." ) ;

	string logtype ;
	string logloc  ;
	string w1      ;
	string w2      ;

	w1 = upper( word( PARM, 1 ) ) ;
	w2 = upper( word( PARM, 2 ) ) ;

	vdefine( "ZCMD ZVERB ZROW1 ZROW2", &zcmd, &zverb, &zrow1, &zrow2 ) ;
	vdefine( "ZAREA ZSHADOW ZAREAT ZSCROLLA", &zarea, &zshadow, &zareat, &zscrolla ) ;
	vdefine( "ZSCROLLN ZAREAW ZAREAD", &zscrolln, &zareaw, &zaread ) ;
	vdefine( "LOGTYPE LOGLOC ZCOL1", &logtype, &logloc, &zcol1 ) ;

	vget( "ZALOG ZSLOG", PROFILE ) ;

	if ( PARM == "AL" )
	{
		logtype = "Application" ;
		vcopy( "ZALOG", logloc, MOVE ) ;
		show_log( logloc ) ;
	}
	else if ( PARM == "SL" )
	{
		logtype = "LSPF"  ;
		vcopy( "ZSLOG", logloc, MOVE ) ;
		show_log( logloc ) ;
	}
	else if ( w1   == "DSL"     ) { dsList( word( PARM, 2 ) ) ; }
	else if ( PARM == "GOPTS"   ) { lspfSettings()       ; }
	else if ( PARM == "KEYS"    ) { pfkeySettings()      ; }
	else if ( PARM == "COLOURS" ) { colourSettings()     ; }
	else if ( PARM == "GCL"     ) { globalColours()      ; }
	else if ( PARM == "TODO"    ) { todoList()           ; }
	else if ( w1   == "VARS"    ) { poolVariables( w2 )  ; }
	else if ( PARM == "PATHS"   ) { showPaths()          ; }
	else if ( PARM == "CMDS"    ) { showCommandTables()  ; }
	else if ( PARM == "MODS"    ) { showLoadedClasses()  ; }
	else if ( w1   == "RUN"     ) { runApplication( w2 ) ; }
	else if ( PARM == "SAVELST" ) { showSavedFileList()  ; }
	else if ( PARM == "TASKS"   ) { showTasks()          ; }
	else if ( PARM == "UTPGMS"  ) { utilityPrograms() ; }
	else if ( PARM == "KLISTS"  ) { keylistTables()   ; }
	else if ( PARM == "KLIST"   ) { keylistTable()    ; }
	else if ( PARM == "CTLKEYS" ) { controlKeys()     ; }
	else if ( PARM == "LIBDEFS" ) { libdefStatus()    ; }
	else if ( w1   == "SETVAR"  )
	{
		vreplace( w2, word( PARM, 3 ) ) ;
		vput( w2, PROFILE ) ;
	}
	else { llog( "E", "Invalid parameter passed to PPSP01A: " << PARM << endl ) ; }

	cleanup() ;
	return    ;
}


void PPSP01A::show_log( const string& fileName )
{
	int fsize ;
	uint t    ;

	bool rebuildZAREA ;

	string Rest  ;
	string Restu ;
	string msg   ;
	string w1    ;
	string w2    ;
	string w3    ;
	string ffilter ;

	filteri = ' '  ;
	filterx = ' '  ;
	ffilter = ""   ;
	firstLine = 0  ;
	maxCol    = 1  ;

	set_apphelp( "HPSP01A" ) ;

	vget( "ZSCRMAXW ZSCRMAXD", SHARED ) ;
	pquery( "PPSP01AL", "ZAREA", "ZAREAT", "ZAREAW", "ZAREAD" ) ;

	zasize   = zareaw * zaread  ;
	startCol = 48     ;
	task     = 0      ;
	showDate = false  ;
	showTime = true   ;
	showMod  = true   ;
	showTask = true   ;

	read_file( fileName ) ;
	fill_dynamic_area()   ;
	rebuildZAREA = false  ;
	Xon          = false  ;
	msg          = ""     ;

	fsize = 0  ;
	file_has_changed( fileName, fsize ) ;

	while ( true )
	{
		zcol1 = d2ds( startCol-47, 7 ) ;
		zrow1 = d2ds( firstLine, 8 )   ;
		zrow2 = d2ds( maxLines, 8 )    ;
		if ( msg == "" ) { zcmd = "" ; }

		display( "PPSP01AL", msg, "ZCMD" ) ;
		if ( RC == 8 ) { return ; }

		vget( "ZVERB ZSCROLLA ZSCROLLN", SHARED ) ;

		msg   = "" ;
		Restu = subword( zcmd, 2 ) ;
		iupper( ( zcmd ) )  ;
		w1    = word( zcmd, 1 )    ;
		w2    = word( zcmd, 2 )    ;
		w3    = word( zcmd, 3 )    ;
		Rest  = subword( zcmd, 2 ) ;

		if ( file_has_changed( fileName, fsize ) )
		{
			read_file( fileName ) ;
			if ( ffilter != "" ) { find_lines( ffilter ) ; }
			rebuildZAREA = true ;
			set_excludes() ;
		}

		if ( w1 == "" )
		{
			;
		}
		else if ( w1 == "ONLY" || w1 == "O" )
		{
			if ( Rest.size() == 1 )
			{
				filteri = Rest[ 0 ] ;
				set_excludes() ;
				rebuildZAREA = true ;
			}
			else { msg = "PPSP012" ; }
		}
		else if ( findword( w1, "TASK TASKID ID" ) )
		{
			if ( w2 == "" )
			{
				task = 0 ;
			}
			else
			{
				task = ds2d ( w2 ) ;
			}
			excluded.clear() ;
			for ( t = 0 ; t <= maxLines ; t++ ) { excluded.push_back( false ) ; }
			set_excludes() ;
			rebuildZAREA = true ;
		}
		else if ( w1 == "EX" || w1 == "X" )
		{
			if ( Rest == "ALL" )
			{
				exclude_all()   ;
				rebuildZAREA = true ;
			}
			else if ( Rest == "ON" )
			{
				Xon = true     ;
				set_excludes() ;
				rebuildZAREA = true ;
			}
			else if ( Rest.size() == 1 )
			{
				filterx = Rest[ 0 ] ;
				set_excludes() ;
				rebuildZAREA = true ;
			}
			else { msg = "PPSP013" ; }
		}
		else if ( w1 == "F" || w1 == "FIND" )
		{
			  ffilter = Restu       ;
			  find_lines( ffilter ) ;
			  rebuildZAREA = true   ;
		}
		else if ( abbrev( "RESET", w1 ) )
		{
			filteri = ' '    ;
			filterx = ' '    ;
			ffilter = ""     ;
			Xon     = false  ;
			task    = 0      ;
			rebuildZAREA = true ;
			excluded.clear() ;
			for ( t = 0 ; t <= maxLines ; t++ ) { excluded.push_back( false ) ; }
		}
		else if ( abbrev( "REFRESH", w1 ) )
		{
			read_file( fileName ) ;
			rebuildZAREA = true ;
			set_excludes() ;
		}
		else if ( w1 == "SHOW" )
		{
			if      ( w2 == "DATE" ) { showDate = true ; }
			else if ( w2 == "TIME" ) { showTime = true ; }
			else if ( w2 == "MOD"  ) { showMod  = true ; }
			else if ( w2 == "TASK" ) { showTask = true ; }
			else if ( w2 == "ALL"  ) { showDate = true ; showTime = true ; showMod = true ; showTask = true ; }
			rebuildZAREA = true ;
		}
		else if ( w1 == "HIDE" )
		{
			if      ( w2 == "DATE" ) { showDate = false ; }
			else if ( w2 == "TIME" ) { showTime = false ; }
			else if ( w2 == "MOD"  ) { showMod  = false ; }
			else if ( w2 == "TASK" ) { showTask = false ; }
			else if ( w2 == "ALL"  ) { showDate = false ; showTime = false ; showMod = false ; showTask = false ; }
			rebuildZAREA = true ;
		}
		else
		{
			msg = "PPSP011" ;
			continue        ;
		}

		if ( zverb == "DOWN" )
		{
			rebuildZAREA = true ;
			if ( zscrolla == "MAX" )
			{
				firstLine = maxLines - zaread ;
				if ( firstLine < 0 ) firstLine = 0 ;
			}
			else
			{
				t = 0 ;
				for ( ; firstLine < ( maxLines - 1 ) ; firstLine++ )
				{
					if ( excluded[ firstLine ] ) continue ;
					t++ ;
					if ( t > zscrolln ) break ;
				}
			}
		}
		else if ( zverb == "UP" )
		{
			rebuildZAREA = true ;
			if ( zscrolla == "MAX" )
			{
				firstLine = 0 ;
			}
			else
			{
				t = 0 ;
				for ( ; firstLine > 0 ; firstLine-- )
				{
					if ( excluded[ firstLine ] ) continue ;
					t++ ;
					if ( t > zscrolln ) break ;
				}
			}
		}
		else if ( zverb == "LEFT" )
		{
			rebuildZAREA = true ;
			if ( zscrolla == "MAX" )
			{
				startCol = 48 ;
			}
			else
			{
				startCol = startCol - zscrolln ;
				if ( startCol < 48 ) { startCol = 48 ; }
			}
		}
		else if ( zverb == "RIGHT" )
		{
			rebuildZAREA = true ;
			if ( zscrolla == "MAX" )
			{
				startCol = maxCol - zareaw + 41 ;
			}
			else
			{
				if ( zscrolla == "CSR" )
				{
					if ( zscrolln == lprefix )
					{
						startCol += zareaw - lprefix ;
					}
					else
					{
						startCol += zscrolln - lprefix ;
					}
				}
				else
				{
					startCol += zscrolln ;
				}
			}
			if ( startCol < 48 ) { startCol = 48 ; }
		}

		if ( rebuildZAREA ) fill_dynamic_area() ;
		rebuildZAREA = false ;
	}
}


void PPSP01A::read_file( const string& fileName )
{
	string inLine ;
	std::ifstream fin( fileName.c_str() ) ;

	data.clear()  ;

	data.push_back( centre( " Top of Log ", zareaw, '*' ) ) ;
	excluded.push_back( false ) ;
	while ( getline( fin, inLine ) )
	{
		data.push_back( inLine )    ;
		excluded.push_back( false ) ;
		if ( maxCol < inLine.size() ) { maxCol = inLine.size() ; }
	}
	maxCol++ ;
	data.push_back( centre( " Bottom of Log ", zareaw, '*' ) ) ;
	excluded.push_back( false ) ;
	maxLines = data.size() ;
	fin.close() ;
}


bool PPSP01A::file_has_changed( const string& fileName, int& fsize )
{
	struct stat results ;

	lstat( fileName.c_str(), &results ) ;

	if ( fsize == 0 )
	{
		fsize = results.st_size ;
	}
	else if ( fsize != results.st_size )
	{
		fsize = results.st_size ;
		return true ;
	}
	return false ;
}


void PPSP01A::set_excludes()
{
	int j ;
	for ( uint i = 1 ; i < maxLines ; i++ )
	{
		if ( task > 0 )
		{
			j = ds2d( data[ i ].substr( 39, 5 ) ) ;
			if ( j != 0 && j != task ) { excluded[ i ] = true ; continue ; }
		}
		if ( Xon )
		{
			if ( data[ i ][ 45 ] == 'D' ||
			     data[ i ][ 45 ] == 'I' ||
			     data[ i ][ 45 ] == '-' )
			{
				excluded[ i ] = true ;
				continue ;
			}
		}
		if ( data[ i ].size() > 45 )
		{
			if ( filteri != ' ' && filteri != data[ i ][ 45 ] && data[ i ][ 45 ] != '*' ) { excluded[ i ] = true ; continue ; }
			if ( filterx != ' ' && filterx == data[ i ][ 45 ] && data[ i ][ 45 ] != '*' ) { excluded[ i ] = true ; continue ; }
		}
	}

}


void PPSP01A::exclude_all()
{
	for ( uint i = 1 ; i < (maxLines-1) ; i++ ) { excluded[ i ] = true ; }
}


void PPSP01A::find_lines( string fnd )
{
	iupper( fnd ) ;
	for ( uint i = 1 ; i < (maxLines-1) ; i++ )
	{
		if ( upper( data[ i ] ).find( fnd ) == string::npos ) { excluded[ i ] = true ; }
	}
}


void PPSP01A::fill_dynamic_area()
{
	int p ;

	string s     ;
	string s2( zareaw, N_TURQ ) ;
	string t     ;

	zarea   = "" ;
	zshadow = "" ;
	zarea.reserve( zasize )   ;
	zshadow.reserve( zasize ) ;

	int l = 0 ;

	for( unsigned int k = firstLine ; k < data.size() ; k++ )
	{
		if ( excluded[ k ] ) continue ;
		l++ ;
		if ( l > zaread ) break ;
		if ( k == 0 || k == maxLines-1 )
		{
			zarea   += substr( data[ k ], 1, zareaw ) ;
			zshadow += s2 ;
		}
		else
		{
			t = "" ;
			s = string( zareaw, N_GREEN ) ;
			p = 0 ;
			if ( showDate ) { t = data[ k ].substr( 0,  12 )     ; s.replace( p, 12, 12, N_TURQ   ) ; p = 12     ; }
			if ( showTime ) { t = t + data[ k ].substr( 12, 16 ) ; s.replace( p, 16, 16, N_TURQ   ) ; p = p + 16 ; }
			if ( showMod  ) { t = t + data[ k ].substr( 28, 11 ) ; s.replace( p, 11, 11, N_YELLOW ) ; p = p + 11 ; }
			t += data[ k ].substr( 39, 8 ) ;
			t += substr( data[ k ], startCol, zareaw ) ;
			t.resize( zareaw, ' ' ) ;
			zarea += t ;
			s.replace( p, 5, 5, N_WHITE ) ;
			p += 6 ;
			s.replace( p, 2, 2, N_TURQ  ) ;
			zshadow += s ;
			lprefix  = p + 2 ;
		}
	}
	zarea.resize( zasize, ' ' ) ;
	zshadow.resize( zasize, N_TURQ ) ;
}


void PPSP01A::dsList( string parms )
{
	// If no parms passed, show the Personal File List screen
	// If parm is a Personal File List, get list of files and invoke PFLSTPGM with LIST
	// Else assume passed parm is a path to be listed

	int i ;

	bool reflist = false ;

	string pgm   ;
	string uprof ;
	string rfltable ;

	string fname ;

	if ( parms == "" )
	{
		vcopy( "ZRFLPGM", pgm, MOVE ) ;
		select( "PGM(" + pgm + ") PARM(PL3) SCRNAME(DSLIST) SUSPEND" ) ;
	}
	else if ( parms.front() == '/' )
	{
		vcopy( "ZFLSTPGM", pgm, MOVE ) ;
		select( "PGM(" + pgm + ") PARM(BROWSE "+parms+") SCRNAME(FILES) SUSPEND" ) ;
	}
	else
	{
		vcopy( "ZUPROF", uprof, MOVE ) ;
		vcopy( "ZRFLTBL", rfltable, MOVE ) ;
		tbopen( rfltable, NOWRITE, uprof ) ;
		if ( RC == 0 )
		{
			vreplace( "ZCURTB", upper( parms ) ) ;
			tbget( rfltable ) ;
			reflist = ( RC == 0 ) ;
			if ( RC == 8 )
			{
				tbsort( rfltable, "(ZCURTB,C,A)" ) ;
				tbvclear( rfltable ) ;
				vreplace( "ZCURTB", upper( parms )+"*" ) ;
				tbsarg( rfltable, "", "NEXT", "(ZCURTB,EQ)" ) ;
				tbscan( rfltable ) ;
				reflist = ( RC == 0 ) ;
				vcopy( "ZCURTB", parms, MOVE ) ;
			}
		}
		if ( reflist )
		{
			std::ofstream fout ;
			vcopy( "ZUSER", zuser, MOVE )     ;
			vcopy( "ZSCREEN", zscreen, MOVE ) ;
			boost::filesystem::path temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;
			string tname = temp.native() ;
			fout.open( tname ) ;
			for ( i = 1 ; i <= 30 ; i++ )
			{
				vcopy( "FLAPET" + d2ds( i, 2 ), fname, MOVE ) ;
				if ( fname == "" ) { continue ; }
				fout << fname << endl ;
			}
			fout.close() ;
			tbclose( rfltable, "", uprof ) ;
			vcopy( "ZFLSTPGM", pgm, MOVE ) ;
			select( "PGM(" + pgm + ") PARM(LIST " + upper( parms ) + " " + tname + ")" ) ;
		}
		else
		{
			tbclose( rfltable, "", uprof ) ;
			vcopy( "ZFLSTPGM", pgm, MOVE ) ;
			select( "PGM(" + pgm + ") PARM(" + parms + ")" ) ;
		}
	}
}


void PPSP01A::lspfSettings()
{
	int timeOut    ;

	string nulls   ;

	string* t1     ;

	string godel   ;
	string goswap  ;
	string goswapc ;
	string gokluse ;
	string goklfal ;
	string golmsgw ;
	string gopadc  ;
	string gosretp ;
	string goscrld ;

	string rodel   ;
	string roswap  ;
	string roswapc ;
	string rokluse ;
	string roklfal ;
	string rolmsgw ;
	string ropadc  ;
	string rosretp ;
	string roscrld ;

	string zdel    ;
	string zswap   ;
	string zswapc  ;
	string zkluse  ;
	string zklfail ;
	string zlmsgw  ;
	string zpadc   ;
	string zsretp  ;
	string zscrolld;

	string goucmd1 ;
	string goucmd2 ;
	string goucmd3 ;
	string goscmd1 ;
	string goscmd2 ;
	string goscmd3 ;
	string gostfst ;

	string roucmd1 ;
	string roucmd2 ;
	string roucmd3 ;
	string roscmd1 ;
	string roscmd2 ;
	string roscmd3 ;
	string rostfst ;

	string goatimo  ;
	string roatimo  ;
	string kmaxwait ;

	string gortsize ;
	string gorbsize ;
	string rortsize ;
	string rorbsize ;
	string zrtsize  ;
	string zrbsize  ;

	string zucmdt1 ;
	string zucmdt2 ;
	string zucmdt3 ;
	string zscmdt1 ;
	string zscmdt2 ;
	string zscmdt3 ;
	string zscmdtf ;

	nulls = string( 1, 0x00 ) ;

	vdefine( "ZUCMDT1 ZUCMDT2 ZUCMDT3", &zucmdt1, &zucmdt2, &zucmdt3 ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ZSCMDT1 ZSCMDT2 ZSCMDT3 ZSCMDTF", &zscmdt1, &zscmdt2, &zscmdt3, &zscmdtf ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "GOUCMD1 GOUCMD2 GOUCMD3", &goucmd1, &goucmd2, &goucmd3 ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ROUCMD1 ROUCMD2 ROUCMD3", &roucmd1, &roucmd2, &roucmd3 ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "GOSCMD1 GOSCMD2 GOSCMD3 GOSTFST", &goscmd1, &goscmd2, &goscmd3, &gostfst ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ROSCMD1 ROSCMD2 ROSCMD3 ROSTFST", &roscmd1, &roscmd2, &roscmd3, &rostfst ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ZKLUSE  ZKLFAIL GOKLUSE GOKLFAL", &zkluse, &zklfail, &gokluse, &goklfal ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ROKLUSE ROKLFAL", &rokluse, &roklfal ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ZDEL    GODEL   ZLMSGW  GOLMSGW", &zdel, &godel, &zlmsgw, &golmsgw ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "RODEL ROLMSGW", &rodel, &rolmsgw ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ZSWAP   GOSWAP   ZSWAPC  GOSWAPC",  &zswap, &goswap, &zswapc, &goswapc ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ROSWAP  ROSWAPC",  &roswap, &roswapc ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ZRTSIZE GORTSIZE ZRBSIZE GORBSIZE", &zrtsize, &gortsize, &zrbsize, &gorbsize ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "RORTSIZE RORBSIZE", &rortsize, &rorbsize ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ZMAXWAIT GOATIMO ZPADC GOPADC", &kmaxwait, &goatimo, &zpadc, &gopadc ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "ROATIMO ROPADC ZSRETP GOSRETP", &roatimo, &ropadc, &zsretp, &gosretp ) ;
	if ( RC > 0 ) { abend() ; }
	vdefine( "GOSCRLD ROSCRLD ZSCROLLD", &goscrld, &roscrld, &zscrolld ) ;
	if ( RC > 0 ) { abend() ; }

	vget( "ZUCMDT1 ZUCMDT2 ZUCMDT3", PROFILE ) ;
	if ( RC > 0 ) { abend() ; }
	vget( "ZSCMDT1 ZSCMDT2 ZSCMDT3 ZSCMDTF", PROFILE ) ;
	if ( RC > 0 ) { abend() ; }
	vget( "ZSWAP ZSWAPC ZKLUSE ZKLFAIL ZSCROLLD", PROFILE ) ;
	if ( RC > 0 ) { abend() ; }
	vget( "ZDEL ZKLUSE  ZLMSGW ZPADC ZSRETP", PROFILE ) ;
	if ( RC > 0 ) { abend() ; }

	gokluse = zkluse  == "Y" ? "/" : "" ;
	goklfal = zklfail == "Y" ? "/" : "" ;
	gostfst = zscmdtf == "Y" ? "/" : "" ;
	golmsgw = zlmsgw  == "Y" ? "/" : "" ;
	goswap  = zswap   == "Y" ? "/" : "" ;
	gosretp = zsretp  == "Y" ? "/" : "" ;

	godel   = zdel   ;
	goswapc = zswapc ;
	if      ( zpadc == " "   ) { zpadc = "B" ; }
	else if ( zpadc == nulls ) { zpadc = "N" ; }
	gopadc  = zpadc    ;
	goscrld = zscrolld ;

	goucmd1 = zucmdt1 ;
	goucmd2 = zucmdt2 ;
	goucmd3 = zucmdt3 ;
	goscmd1 = zscmdt1 ;
	goscmd2 = zscmdt2 ;
	goscmd3 = zscmdt3 ;

	vcopy( "ZWAIT", t1, LOCATE ) ;
	vget( "ZMAXWAIT ZRTSIZE ZRBSIZE", PROFILE ) ;

	timeOut = ds2d( *t1 ) * ds2d( kmaxwait ) / 1000 ;
	goatimo = d2ds( timeOut ) ;

	gortsize = zrtsize ;
	gorbsize = zrbsize ;

	rodel    = godel    ;
	roswap   = goswap   ;
	roswapc  = goswapc  ;
	rokluse  = gokluse  ;
	roklfal  = goklfal  ;
	rolmsgw  = golmsgw  ;
	ropadc   = gopadc   ;
	roscrld  = goscrld  ;
	rosretp  = gosretp  ;
	roucmd1  = goucmd1  ;
	roucmd2  = goucmd2  ;
	roucmd3  = goucmd3  ;
	roscmd1  = goscmd1  ;
	roscmd2  = goscmd2  ;
	roscmd3  = goscmd3  ;
	rostfst  = gostfst  ;
	roatimo  = goatimo  ;
	rortsize = gortsize ;
	rorbsize = gorbsize ;

	while ( true )
	{
		zcmd = "" ;
		display( "PPSP01GO", "", "ZCMD" );
		if ( RC == 8 ) { break ; }
		if ( zcmd == "DEFAULTS" )
		{
			gokluse  = ""  ;
			goklfal  = "/" ;
			gostfst  = "/" ;
			golmsgw  = ""  ;
			godel    = ";" ;
			goswap   = "/" ;
			goswapc  = "'" ;
			gopadc   = "_" ;
			goscrld  = "HALF" ;
			gosretp  = ""  ;
			goucmd1  = "USR"  ;
			goucmd2  = ""  ;
			goucmd3  = ""  ;
			goscmd1  = ""  ;
			goscmd2  = ""  ;
			goscmd3  = ""  ;
			goatimo  = d2ds( ZMAXWAIT * ds2d( *t1 ) / 1000 ) ;
			gortsize = "3"  ;
			gorbsize = "20" ;
		}
		else if ( zcmd == "RESET" )
		{
			godel    = rodel    ;
			goswap   = roswap   ;
			goswapc  = roswapc  ;
			gokluse  = rokluse  ;
			goklfal  = roklfal  ;
			golmsgw  = rolmsgw  ;
			gopadc   = ropadc   ;
			goscrld  = roscrld  ;
			gosretp  = rosretp  ;
			goucmd1  = roucmd1  ;
			goucmd2  = roucmd2  ;
			goucmd3  = roucmd3  ;
			goscmd1  = roscmd1  ;
			goscmd2  = roscmd2  ;
			goscmd3  = roscmd3  ;
			gostfst  = rostfst  ;
			goatimo  = roatimo  ;
			gortsize = rortsize ;
			gorbsize = rorbsize ;
		}
		zkluse   = gokluse == "/" ? "Y" : "N" ;
		zklfail  = goklfal == "/" ? "Y" : "N" ;
		zscmdtf  = gostfst == "/" ? "Y" : "N" ;
		zlmsgw   = golmsgw == "/" ? "Y" : "N" ;
		zswap    = goswap  == "/" ? "Y" : "N" ;
		zsretp   = gosretp == "/" ? "Y" : "N" ;
		zucmdt1  = goucmd1 ;
		zucmdt2  = goucmd2 ;
		zucmdt3  = goucmd3 ;
		zscmdt1  = goscmd1 ;
		zscmdt2  = goscmd2 ;
		zscmdt3  = goscmd3 ;
		zscrolld = goscrld ;
		vput( "ZKLUSE  ZKLFAIL ZLMSGW  ZSWAP   ZSRETP", PROFILE ) ;
		vput( "ZUCMDT1 ZUCMDT2 ZUCMDT3", PROFILE ) ;
		vput( "ZSCMDT1 ZSCMDT2 ZSCMDT3 ZSCMDTF ZSCROLLD", PROFILE ) ;
		if ( godel != "" && godel != zdel )
		{
			zdel = godel ;
			vput( "ZDEL", PROFILE ) ;
		}
		if ( goswapc != "" && goswapc != zswapc )
		{
			zswapc = goswapc ;
			vput( "ZSWAPC", PROFILE ) ;
		}
		if ( gopadc != zpadc )
		{
			if      ( gopadc == "B" ) { zpadc = " "    ; }
			else if ( gopadc == "N" ) { zpadc = nulls  ; }
			else                      { zpadc = gopadc ; }
			vput( "ZPADC", PROFILE ) ;
			zpadc = gopadc ;
		}
		if ( goatimo != "" )
		{
			kmaxwait = d2ds( ds2d( goatimo ) * 1000 / ds2d( *t1 ) ) ;
			vput( "ZMAXWAIT", PROFILE ) ;
		}
		if ( gortsize != "" )
		{
			zrtsize = gortsize ;
			vput( "ZRTSIZE", PROFILE ) ;
		}
		if ( gorbsize != "" )
		{
			zrbsize = gorbsize ;
			vput( "ZRBSIZE", PROFILE ) ;
		}
	}
	vdelete( "ZUCMDT1 ZUCMDT2 ZUCMDT3" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "ZSCMDT1 ZSCMDT2 ZSCMDT3 ZSCMDTF" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "GOUCMD1 GOUCMD2 GOUCMD3" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "GOSCMD1 GOSCMD2 GOSCMD3 GOSTFST" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "ZKLUSE  ZKLFAIL GOKLUSE GOKLFAL" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "ZDEL    GODEL   ZLMSGW  GOLMSGW"  ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "ZSWAP   GOSWAP  ZSWAPC  GOSWAPC"  ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "ZRTSIZE GORTSIZE ZRBSIZE GORBSIZE" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "ZMAXWAIT GOATIMO ZPADC GOPADC ZSRETP GOSRETP" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "RODEL ROSWAP ROSWAPC ROKLUSE ROKLFAL ROLMSGW ROPADC ROUCMD1 ROUCMD2" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "ROUCMD3 ROSCMD1 ROSCMD2 ROSCMD3 ROSTFST ROATIMO RORTSIZE RORBSIZE" ) ;
	if ( RC > 0 ) { abend() ; }
	vdelete( "GOSCRLD ROSCRLD ZSCROLLD" ) ;
	if ( RC > 0 ) { abend() ; }
}


void PPSP01A::controlKeys()
{
	int i ;

	bool e_loop  ;

	string key1  ;
	string key2  ;
	string table ;
	string zcmd  ;
	string msg   ;

	string* t1   ;
	string* t2   ;

	const string alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ;
	vector<string>oldValues ;

	vdefine( "ZCMD", &zcmd ) ;
	table = "CTKEYS" + d2ds( taskid(), 2 ) ;

	tbcreate( table, "CTKEY1", "(CTKEY2,CTACT)", NOWRITE ) ;

	for ( i = 0 ; i < 26 ; i++ )
	{
		key1 = "ZCTRL"    ;
		key2 = "Control-" ;
		key1.push_back( alpha[ i ] ) ;
		key2.push_back( alpha[ i ] ) ;
		vcopy( key1, t1, LOCATE )  ;
		oldValues.push_back( *t1 ) ;
		vreplace( "CTKEY1", key1 ) ;
		vreplace( "CTKEY2", key2 ) ;
		vreplace( "CTACT", *t1 ) ;
		tbadd( table ) ;
	}

	ztdtop = 1  ;
	msg    = "" ;
	e_loop = false ;
	while ( true )
	{
		tbtop( table ) ;
		tbskip( table, ztdtop ) ;
		zcmd = "" ;
		tbdispl( table, "PPSP01CT", msg, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		if ( zcmd == "RESET" || zcmd == "CANCEL" )
		{
			for ( i = 0 ; i < 26 ; i++ )
			{
				key1 = "ZCTRL" ;
				key2 = "Control-" ;
				key1.push_back( alpha[ i ] ) ;
				key2.push_back( alpha[ i ] ) ;
				vreplace( "CTKEY1", key1 ) ;
				vreplace( "CTKEY2", key2 ) ;
				vreplace( "CTACT", oldValues[ i ] ) ;
				tbmod( table ) ;
				vreplace( key1, oldValues[ i ] ) ;
				vput( key1, PROFILE ) ;
			}
			if ( zcmd == "CANCEL" ) { break ; }
			continue ;
		}
		if ( zcmd == "RESTORE" )
		{
			msg = "PPSP011E" ;
			for ( i = 0 ; i < 26 ; i++ )
			{
				key1 = "ZCTRL" ;
				key2 = "ACTRL" ;
				key1.push_back( alpha[ i ] ) ;
				key2.push_back( alpha[ i ] ) ;
				vget( key2, PROFILE ) ;
				if ( RC > 0 )
				{
					msg = "PPSP011C" ;
					break ;
				}
				vcopy( key2, t1, LOCATE ) ;
				vreplace( key1, *t1 ) ;
				vput( key1, PROFILE ) ;
				key2 = "Control-" ;
				key2.push_back( alpha[ i ] ) ;
				vreplace( "CTKEY1", key1 ) ;
				vreplace( "CTKEY2", key2 ) ;
				vreplace( "CTACT", *t1 ) ;
				tbmod( table ) ;
			}
			continue ;
		}
		while ( ztdsels > 0 )
		{
			vcopy( "CTKEY1", t1, LOCATE ) ;
			vcopy( "CTACT", t2, LOCATE ) ;
			vreplace( *t1, *t2 ) ;
			vput( *t1, PROFILE ) ;
			tbmod( table ) ;
			if ( ztdsels > 1 )
			{
				tbdispl( table ) ;
				if ( RC > 4 ) { e_loop = true ; break ; }
			}
			else { ztdsels = 0 ; }
		}
		if ( e_loop ) { break ; }
		if ( zcmd == "SAVE" )
		{
			for ( i = 0 ; i < 26 ; i++ )
			{
				key1 = "ZCTRL" ;
				key2 = "ACTRL" ;
				key1.push_back( alpha[ i ] ) ;
				key2.push_back( alpha[ i ] ) ;
				vget( key1, PROFILE ) ;
				vcopy( key1, t1, LOCATE ) ;
				vreplace( key2, *t1 ) ;
				vput( key2, PROFILE ) ;
			}
			msg = "PPSP011D" ;
		}
	}

	tbend( table ) ;
	vdelete( "ZCMD" ) ;
}


void PPSP01A::pfkeySettings()
{
	int RCode ;

	string* t ;

	vector<string> pfKeyDefaults = { { "HELP"      },
					 { "SPLIT NEW" },
					 { "END"       },
					 { "RETURN"    },
					 { "RFIND"     },
					 { "RCHANGE"   },
					 { "UP"        },
					 { "DOWN"      },
					 { "SWAP"      },
					 { "LEFT"      },
					 { "RIGHT"     },
					 { "RETRIEVE"  },
					 { "HELP"      },
					 { "SPLIT NEW" },
					 { "END"       },
					 { "RETURN"    },
					 { "RFIND"     },
					 { "RCHANGE"   },
					 { "UP"        },
					 { "DOWN"      },
					 { "SWAP"      },
					 { "SWAP PREV" },
					 { "SWAP NEXT" },
					 { "HELP"      } } ;

	for ( int i = 0 ; i < 24 ; i++ )
	{
		vget( "ZPF"+d2ds( i+1, 2 ), PROFILE ) ;
	}

	while ( true )
	{
		zcmd  = "" ;
		display( "PPSP01AK", "", "ZCMD" );
		RCode = RC ;

		if ( zcmd == "CANCEL" ) { break ; }
		if ( zcmd == "DEFAULTS" )
		{
			for ( int i = 0 ; i < 24 ; i++ )
			{
				vreplace( "ZPF"+d2ds( i+1, 2 ), "" ) ;
			}
		}

		for ( int i = 0 ; i < 24 ; i++ )
		{
			vcopy( "ZPF"+d2ds( i+1, 2 ), t, LOCATE ) ;
			if ( *t == "" )
			{
				vreplace( "ZPF"+d2ds( i+1, 2 ), pfKeyDefaults[ i ] ) ;
			}
		}

		if ( RCode == 8 || zcmd == "SAVE" )
		{
			for ( int i = 0 ; i < 24 ; i++ )
			{
				vput( "ZPF"+d2ds( i+1, 2 ), PROFILE ) ;
			}
			if ( RCode == 8 )  { break ; }
		}
	}
}


void PPSP01A::colourSettings()
{

	int  i ;

	string msg    ;
	string curfld ;
	string var1   ;
	string var2   ;
	string var3   ;
	string isps_var ;
	string prof_var ;
	string attr1  ;
	string attr2  ;
	string attr3  ;
	string colour ;
	string intens ;
	string hilite ;

	map< int, string>VarList ;
	map< int, string>DefList ;
	map< int, string>OrigList ;

	VarList[ 1  ] = "AB"   ;
	VarList[ 2  ] = "ABSL" ;
	VarList[ 3  ] = "ABU"  ;
	VarList[ 4  ] = "AMT"  ;
	VarList[ 5  ] = "AWF"  ;
	VarList[ 6  ] = "CT"   ;
	VarList[ 7  ] = "CEF"  ;
	VarList[ 8  ] = "CH"   ;
	VarList[ 9  ] = "DT"   ;
	VarList[ 10 ] = "ET"   ;
	VarList[ 11 ] = "EE"   ;
	VarList[ 12 ] = "FP"   ;
	VarList[ 13 ] = "FK"   ;
	VarList[ 14 ] = "IMT"  ;
	VarList[ 15 ] = "LEF"  ;
	VarList[ 16 ] = "LID"  ;
	VarList[ 17 ] = "LI"   ;
	VarList[ 18 ] = "NEF"  ;
	VarList[ 19 ] = "NT"   ;
	VarList[ 20 ] = "PI"   ;
	VarList[ 21 ] = "PIN"  ;
	VarList[ 22 ] = "PT"   ;
	VarList[ 23 ] = "PS"   ;
	VarList[ 24 ] = "PAC"  ;
	VarList[ 25 ] = "PUC"  ;
	VarList[ 26 ] = "RP"   ;
	VarList[ 27 ] = "SI"   ;
	VarList[ 28 ] = "SAC"  ;
	VarList[ 29 ] = "SUC"  ;
	VarList[ 30 ] = "VOI"  ;
	VarList[ 31 ] = "WMT"  ;
	VarList[ 32 ] = "WT"   ;
	VarList[ 33 ] = "WASL" ;

	DefList[ 1  ] = KAB    ;
	DefList[ 2  ] = KABSL  ;
	DefList[ 3  ] = KABU   ;
	DefList[ 4  ] = KAMT   ;
	DefList[ 5  ] = KAWF   ;
	DefList[ 6  ] = KCT    ;
	DefList[ 7  ] = KCEF   ;
	DefList[ 8  ] = KCH    ;
	DefList[ 9  ] = KDT    ;
	DefList[ 10 ] = KET    ;
	DefList[ 11 ] = KEE    ;
	DefList[ 12 ] = KFP    ;
	DefList[ 13 ] = KFK    ;
	DefList[ 14 ] = KIMT   ;
	DefList[ 15 ] = KLEF   ;
	DefList[ 16 ] = KLID   ;
	DefList[ 17 ] = KLI    ;
	DefList[ 18 ] = KNEF   ;
	DefList[ 19 ] = KNT    ;
	DefList[ 20 ] = KPI    ;
	DefList[ 21 ] = KPIN   ;
	DefList[ 22 ] = KPT    ;
	DefList[ 23 ] = KPS    ;
	DefList[ 24 ] = KPAC   ;
	DefList[ 25 ] = KPUC   ;
	DefList[ 26 ] = KRP    ;
	DefList[ 27 ] = KSI    ;
	DefList[ 28 ] = KSAC   ;
	DefList[ 29 ] = KSUC   ;
	DefList[ 30 ] = KVOI   ;
	DefList[ 31 ] = KWMT   ;
	DefList[ 32 ] = KWT    ;
	DefList[ 33 ] = KWASL  ;

	control( "RELOAD", "CUA" ) ;

	attr1 = "" ;
	attr2 = "" ;
	attr3 = "" ;

	control( "DISPLAY", "LOCK" ) ;
	display( "PPSP01CL" )        ;

	for ( i = 1 ; i < 34 ; i++ )
	{
		if ( setScreenAttrs( VarList[ i ], i, colour, intens, hilite ) > 0 )
		{
			llog( "E", "ISPS variable " << "ZC" + VarList[ i ] << " not found.  Re-run setup program to create" << endl ) ;
			abend() ;
		}
	}

	for ( i = 1 ; i < 34 ; i++ )
	{
		isps_var = "ZC" + VarList[i] ;
		vcopy( isps_var, val, MOVE ) ;
		if ( RC > 0 )
		{
			llog( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
			abend() ;
		}
		OrigList[ i ] = val ;
	}

	msg    = "" ;
	zcmd   = "" ;
	while ( true )
	{
		if ( msg == "" ) { curfld = "ZCMD" ; }
		display( "PPSP01CL", msg, curfld ) ;
		if (RC == 8 ) { cleanup() ; break  ; }

		if ( zcmd == "" )
		{
			;
		}
		else if ( zcmd == "CANCEL" )
		{
			for ( i = 1 ; i < 34 ; i++ )
			{
				isps_var = "ZC" + VarList[ i ] ;
				vcopy( isps_var, val, MOVE )   ;
				if ( RC > 0 )
				{
					llog( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
					abend() ;
				}
				setISPSVar( VarList[ i ], OrigList[ i ] ) ;
			}
			return ;
		}
		else if ( zcmd == "DEFAULTS" )
		{
			for ( i = 1 ; i < 34 ; i++ )
			{
				setISPSVar( VarList[ i ], DefList[ i ]  ) ;
			}
			for ( i = 1 ; i < 34 ; i++ )
			{
				setScreenAttrs( VarList[ i ],  i, colour, intens, hilite ) ;
			}
		}
		else if ( zcmd == "SAVE" )
		{
			zcmd = "" ;
			for ( i = 1 ; i < 34 ; i++ )
			{
				isps_var = "ZC" + VarList[i] ;
				vcopy( isps_var, val, MOVE ) ;
				if ( RC > 0 )
				{
					llog( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
					abend() ;
				}
				prof_var      = isps_var  ;
				prof_var[ 0 ] = 'A'       ;
				vdefine( prof_var, &val ) ;
				vput( prof_var, PROFILE ) ;
				vdelete( prof_var ) ;
			}
			if ( RC == 0 ) { msg = "PPSP011A" ; }
			continue ;
		}
		else if ( zcmd == "RESTORE" )
		{
			zcmd = "" ;
			for ( i = 1 ; i < 34 ; i++ )
			{
				prof_var = "AC" + VarList[i] ;
				vcopy( prof_var, val, MOVE ) ;
				if ( RC > 0 ) { msg = "PPSP019" ; break ; }
				isps_var      = prof_var  ;
				isps_var[ 0 ] = 'Z'       ;
				vdefine( isps_var, &val ) ;
				vput( isps_var, PROFILE ) ;
				vdelete( isps_var ) ;
				if ( setScreenAttrs( VarList[i],  i, colour, intens, hilite ) > 0 ) { abend() ; }
			}
			if ( RC == 0 ) { msg = "PPSP011B" ; }
			continue ;
		}

		msg  = "" ;
		zcmd = "" ;
		for ( i = 1 ; i < 34 ; i++)
		{
			var1 = "COLOUR" + d2ds( i, 2 ) ;
			var2 = "INTENS" + d2ds( i, 2 ) ;
			var3 = "HILITE" + d2ds( i, 2 ) ;
			vcopy( var1, colour, MOVE ) ;
			vcopy( var2, intens, MOVE ) ;
			vcopy( var3, hilite, MOVE ) ;
			if ( colour == "" ) { colour = DefList[ i ][ 0 ] ; }
			if ( intens == "" ) { intens = DefList[ i ][ 1 ] ; }
			if ( hilite == "" ) { hilite = DefList[ i ][ 2 ] ; }
			isps_var = "ZC" + VarList[i] ;
			vcopy( isps_var, val, MOVE ) ;
			if ( RC > 0 )
			{
				llog( "E", "ISPS variable " << isps_var << " not found.  Re-run setup program to create" << endl ) ;
				abend() ;
			}
			if ( val.size() != 3 )
			{
				llog( "E", "ISPS variable " << isps_var << " has invalid value of " << val << "  Re-run setup program to re-create" << endl ) ;
				abend() ;
			}
			switch ( colour[ 0 ] )
			{
			case 'R':
				vreplace( var1, "RED" ) ;
				val[ 0 ] = 'R'          ;
				attr1 = "COLOUR(RED)"   ;
				break ;
			case 'G':
				vreplace( var1, "GREEN" ) ;
				val[ 0 ] = 'G'            ;
				attr1 = "COLOUR(GREEN)"   ;
				break ;
			case 'Y':
				vreplace( var1, "YELLOW" ) ;
				val[ 0 ] = 'Y'             ;
				attr1 = "COLOUR(YELLOW)"   ;
				break ;
			case 'B':
				vreplace( var1, "BLUE" ) ;
				val[ 0 ] = 'B'           ;
				attr1 = "COLOUR(BLUE)"   ;
				break ;
			case 'M':
				vreplace( var1, "MAGENTA" ) ;
				val[ 0 ] = 'M'              ;
				attr1 = "COLOUR(MAGENTA)"   ;
				break ;
			case 'T':
				vreplace( var1, "TURQ" ) ;
				val[ 0 ] = 'T'           ;
				attr1 = "COLOUR(TURQ)"   ;
				break ;
			case 'W':
				vreplace( var1, "WHITE" ) ;
				val[ 0 ] = 'W'            ;
				attr1 = "COLOUR(WHITE)"   ;
				break ;
			default:
				msg    = "PPSP016" ;
				curfld = var1      ;
			}
			switch ( intens[ 0 ] )
			{
			case 'H':
				vreplace( var2, "HIGH" ) ;
				val[ 1 ] = 'H'           ;
				attr2 = "INTENSE(HIGH)"  ;
				break ;
			case 'L':
				vreplace( var2, "LOW"  ) ;
				val[ 1 ] = 'L'           ;
				attr2 = "INTENSE(LOW)"   ;
				break ;
			default:
				msg    = "PPSP017" ;
				curfld = var2      ;
			}
			switch ( hilite[ 0 ] )
			{
			case 'N':
				vreplace( var3, "NONE" ) ;
				val[ 2 ] = 'N'           ;
				attr3 = "HILITE(NONE)"   ;
				break ;
			case 'B':
				vreplace( var3, "BLINK" ) ;
				val[ 2 ] = 'B'            ;
				attr3 = "HILITE(BLINK)"   ;
					break ;
			case 'R':
				vreplace( var3, "REVERSE" ) ;
				val[ 2 ] = 'R'              ;
				attr3 = "HILITE(REVERSE)"   ;
				break ;
			case 'U':
				vreplace( var3, "USCORE" ) ;
				val[ 2 ] = 'U'             ;
				attr3 = "HILITE(USCORE)"   ;
				break ;
			default:
				msg    = "PPSP018" ;
				curfld = var3      ;
			}
			vdefine( isps_var, &val ) ;
			vput( isps_var, PROFILE ) ;
			vdelete( isps_var )       ;
			attr( var1, attr1 )       ;
			if ( RC > 0 ) { llog( "E", "Colour change for field " << var1 << " has failed." << endl ) ; }
			attr( var2, attr1 + " " + attr2 ) ;
			if ( RC > 0 ) { llog( "E", "Colour/intense change for field " << var2 << " has failed." << endl ) ; }
			attr( var3, attr1 + " " + attr2 + " " + attr3 ) ;
			if ( RC > 0 ) { llog( "E", "Colour/intense/hilite change for field " << var3 << " has failed." << endl ) ; }
		}
	}
}


void PPSP01A::globalColours()
{
	int i ;

	string colour ;

	map<int, string> tab1 = { { 1, "B" } ,
				  { 2, "R" } ,
				  { 3, "M" } ,
				  { 4, "G" } ,
				  { 5, "T" } ,
				  { 6, "Y" } ,
				  { 7, "W" } } ;

	map<string, string> tab2 = { { "B", "BLUE"    } ,
				     { "R", "RED"     } ,
				     { "M", "MAGENTA" } ,
				     { "G", "GREEN"   } ,
				     { "T", "TURQ"    } ,
				     { "Y", "YELLOW"  } ,
				     { "W", "WHITE"   } } ;

	control( "RELOAD", "CUA" ) ;

	for ( i = 1 ; i < 8 ; i++ )
	{
		var = "ZGCL" + tab1[ i ] ;
		vcopy( var, colour, MOVE ) ;
		if ( colour == tab1[ i ] )
		{
			vreplace( "COLOUR"+ d2ds( i, 2 ), "" ) ;
		}
		else
		{
			vreplace( "COLOUR"+ d2ds( i, 2 ), tab2[ colour ] ) ;
		}
	}

	addpop( "", 5, 5 ) ;
	while ( true )
	{
		display( "PPSP01CR" ) ;
		if (RC == 8 ) { cleanup() ; break ; }
		for ( i = 1 ; i < 8 ; i++ )
		{
			vcopy( "COLOUR"+ d2ds( i, 2 ), colour, MOVE ) ;
			var = "ZGCL" + tab1[ i ] ;
			val = ( colour == "" ) ? tab1[ i ] : colour.substr( 0, 1 ) ;
			vreplace( var, val ) ;
			vput( var, PROFILE ) ;
		}
	}
	rempop() ;
}


int PPSP01A::setScreenAttrs( const string& name, int itr, string colour, string intens, string hilite )
{
	string t ;
	char   c ;

	string var1 ;
	string var2 ;
	string var3 ;

	vcopy( "ZC" + name, t, MOVE ) ;
	if ( RC > 0 ) { llog( "E", "Variable ZC" << name << " not found in ISPS profile" << endl ) ; return 8 ; }
	else
	{
		if ( t.size() != 3 ) { llog( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; return 8 ; }
		c = t[ 0 ] ;
		if      ( c == 'R' ) colour = "RED"     ;
		else if ( c == 'G' ) colour = "GREEN"   ;
		else if ( c == 'Y' ) colour = "YELLOW"  ;
		else if ( c == 'B' ) colour = "BLUE"    ;
		else if ( c == 'M' ) colour = "MAGENTA" ;
		else if ( c == 'T' ) colour = "TURQ"    ;
		else if ( c == 'W' ) colour = "WHITE"   ;
		else { llog( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; }
		c = t[ 1 ] ;
		if      ( c == 'H' ) intens = "HIGH" ;
		else if ( c == 'L' ) intens = "LOW" ;
		else { llog( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; }
		c = t[ 2 ] ;
		if      ( c == 'N' ) hilite = "NONE"    ;
		else if ( c == 'B' ) hilite = "BLINK"   ;
		else if ( c == 'R' ) hilite = "REVERSE" ;
		else if ( c == 'U' ) hilite = "USCORE"  ;
		else { llog( "E", "Variable ZC" << name << " has invalid value " << t << endl ) ; }
	}

	var1 = "COLOUR" + d2ds( itr, 2 ) ;
	var2 = "INTENS" + d2ds( itr, 2 ) ;
	var3 = "HILITE" + d2ds( itr, 2 ) ;

	attr( var1, "COLOUR(" + colour + ")" ) ;
	if ( RC > 0 ) { llog( "E", "Colour change for field " << var1 << " has failed." << endl ) ; }

	attr( var2, "COLOUR(" + colour + ") INTENSE(" + intens + ")" ) ;
	if ( RC > 0 ) { llog( "E", "Colour/intense change for field " << var2 << " has failed." << endl ) ; }

	attr( var3,  "COLOUR(" + colour + ") INTENSE(" + intens + ") HILITE(" + hilite + ")" ) ;
	if ( RC > 0 ) { llog( "E", "Colour/intense/hilite change for field " << var3 << " has failed." << endl ) ; }

	vreplace( var1, colour ) ;
	vreplace( var2, intens ) ;
	vreplace( var3, hilite ) ;
	return 0 ;
}


void PPSP01A::setISPSVar( const string& xvar, string xval )
{
	string isps_var ;

	isps_var = "ZC" + xvar     ;
	vdefine( isps_var, &xval ) ;
	vput( isps_var, PROFILE )  ;
	vdelete( isps_var )        ;
}


void PPSP01A::todoList()
{
	addpop( "", 5, 5 ) ;
	while ( true )
	{
		display( "PPSP01TD", "", "ZCMD" );
		if ( RC == 8 ) { break ; }
	}
	rempop() ;
}


void PPSP01A::poolVariables( const string& applid )
{
	string msg   ;
	string cw    ;
	string w2    ;
	string table ;

	bool e_loop  ;

	vcopy( "ZAPPLID", zapplid, MOVE ) ;
	if ( applid != "" && zapplid != applid )
	{
		if ( !isvalidName4( applid ) ) { return ; }
		select( "PGM(PPSP01A) PARM(VARS) NEWAPPL(" + applid + ")" ) ;
		return ;
	}

	table = "VARLST" + d2ds( taskid(), 2 ) ;

	vdefine( "SEL VAR VPOOL VPLVL VAL MESSAGE", &sel, &var, &vpool, &vplvl, &val, &message ) ;

	getpoolVariables( table, "" ) ;

	msg    = "" ;
	ztdtop = 1  ;
	e_loop = false ;
	while ( true )
	{
		tbtop( table )  ;
		tbskip( table, ztdtop ) ;
		if ( msg == "" ) { zcmd = "" ; }
		tbdispl( table, "PPSP01AV", msg, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		if ( zcmd == "REF" || zcmd == "RES" )
		{
			tbend( table ) ;
			getpoolVariables( table, "" ) ;
			continue ;
		}
		cw = word( zcmd, 1 ) ;
		w2 = word( zcmd, 2 ) ;
		if ( cw == "O" )
		{
			tbend( table ) ;
			getpoolVariables( table, w2 ) ;
			continue ;
		}
		while ( ztdsels > 0 )
		{
			if ( sel == "D" )
			{
				control( "ERRORS", "RETURN" ) ;
				if ( vpool == "S" ) { verase( var, SHARED  ) ; }
				else                { verase( var, PROFILE ) ; }
				message = "*Delete RC=" + d2ds(RC) + "*" ;
				control( "ERRORS", "CANCEL" ) ;
				sel = "" ;
			}
			tbput( table ) ;
			if ( ztdsels > 1 )
			{
				tbdispl( table ) ;
				if ( RC > 4 ) { e_loop = true ; break ; }
			}
			else { ztdsels = 0 ; }
		}
		if ( e_loop ) { break ; }
	}

	tbend( table ) ;
}


void PPSP01A::getpoolVariables( const string& table, const string& pattern )
{
	// SHARED 1  - shared variable pool
	// SHARED 2  - default variable pool (@DEFSHAR)
	// PROFILE 1 - application variable pool
	// PROFILE 2 - Read-only extention pool
	// PROFILE 3 - default read-only profile pool (@DEFPROF)
	// PROFILE 4 - System profile (ISPSPROF)

	int i  ;
	int ws ;

	string varlist ;

	tbcreate( table, "", "(SEL,VAR,VPOOL,VPLVL,MESSAGE,VAL)", NOWRITE ) ;

	sel     = "" ;
	message = "" ;

	set<string> found ;
  /*    varlist = vilist( DEFINED ) + vslist( DEFINED ) ;
	vpool = "F" ;
	vplvl = "D" ;
	ws    = words( varlist ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		var = word( varlist, i ) ;
		if ( (pattern != "") && (pos( pattern, var ) == 0) ) { continue ; }
		vcopy( var, val, MOVE ) ;
		tbadd( table )    ;
	}

	varlist = vilist( IMPLICIT ) + vslist( IMPLICIT ) ;
	vpool = "F" ;
	vplvl = "I" ;
	ws    = words( varlist ) ;
	for ( i = 1 ; i <= ws ; i++ )
	{
		var = word( varlist, i ) ;
		if ( (pattern != "") && (pos( pattern, var ) == 0) ) { continue ; }
		vcopy( var, val, MOVE ) ;
		tbadd( table )    ;
	}
	*/
	found.clear() ;
	varlist = vlist( SHARED, 1 ) ;
	vpool   = "S" ;
	vplvl   = "1" ;
	for ( ws = words( varlist ), i = 1 ; i <= ws ; i++ )
	{
		var = word( varlist, i ) ;
		if ( pattern != "" && pos( pattern, var ) == 0 ) { continue ; }
		vget( var, SHARED ) ;
		vcopy( var, val, MOVE ) ;
		tbadd( table )      ;
		found.insert( var ) ;
	}

	varlist = vlist( SHARED, 2 ) ;
	vplvl   = "2" ;
	for ( ws = words( varlist ), i = 1 ; i <= ws ; i++ )
	{
		var = word( varlist, i ) ;
		if ( found.count( var ) > 0 ) { continue ; }
		if ( pattern != "" && pos( pattern, var ) == 0 ) { continue ; }
		vget( var, SHARED ) ;
		vcopy( var, val, MOVE ) ;
		tbadd( table )      ;
		found.insert( var ) ;
	}

	found.clear() ;
	vpool   = "P" ;
	vplvl   = "1" ;
	varlist = vlist( PROFILE, 1 ) ;
	for ( ws = words( varlist ), i = 1 ; i <= ws ; i++ )
	{
		var = word( varlist, i ) ;
		if ( pattern != "" && pos( pattern, var ) == 0 ) { continue ; }
		vget( var, PROFILE ) ;
		vcopy( var, val, MOVE ) ;
		tbadd( table )      ;
		found.insert( var ) ;
	}

	varlist = vlist( PROFILE, 2 ) ;
	vplvl   = "2"                 ;
	for ( ws = words( varlist ), i = 1 ; i <= ws ; i++ )
	{
		var = word( varlist, i ) ;
		if ( found.count( var ) > 0 ) { continue ; }
		if ( pattern != "" && pos( pattern, var ) == 0 ) { continue ; }
		vget( var, PROFILE ) ;
		vcopy( var, val, MOVE ) ;
		tbadd( table )      ;
		found.insert( var ) ;
	}

	varlist = vlist( PROFILE, 3 ) ;
	vplvl   = "3"                 ;
	for ( ws = words( varlist ), i = 1 ; i <= ws ; i++ )
	{
		var = word( varlist, i ) ;
		if ( found.count( var ) > 0 ) { continue ; }
		if ( pattern != "" && pos( pattern, var ) == 0 ) { continue ; }
		vget( var, PROFILE ) ;
		vcopy( var, val, MOVE ) ;
		tbadd( table )      ;
		found.insert( var ) ;
	}

	varlist = vlist( PROFILE, 4 ) ;
	vplvl   = "4"                 ;
	for ( ws = words( varlist ), i = 1 ; i <= ws ; i++ )
	{
		var = word( varlist, i ) ;
		if ( found.count( var ) > 0 ) { continue ; }
		if ( pattern != "" && pos( pattern, var ) == 0 ) { continue ; }
		vget( var, PROFILE ) ;
		vcopy( var, val, MOVE ) ;
		tbadd( table ) ;
	}

	tbtop( table ) ;
}


void PPSP01A::showPaths()
{
	// Display search paths for messages, panels, tables, REXX, profiles and load modules
	// Include LIBDEF entries

	int i ;

	bool e_loop     ;

	string zmlib    ;
	string zplib    ;
	string ztlib    ;
	string zorxpath ;
	string zldpath  ;
	string libs     ;

	string pgm      ;
	string table    ;
	string pvar     ;
	string path     ;
	string desc     ;
	string msg      ;

	vdefine( "LIBS", &libs ) ;

	table = "PTHLST" + d2ds( taskid(), 2 ) ;

	vdefine( "SEL PVAR PATH MESSAGE DESC", &sel, &pvar, &path, &message, &desc ) ;

	tbcreate( table, "", "(SEL,PVAR,PATH,MESSAGE,DESC)", NOWRITE ) ;

	vget( "ZMLIB ZPLIB ZTLIB ZORXPATH ZLDPATH", PROFILE ) ;

	vcopy( "ZMLIB", zmlib, MOVE ) ;
	vcopy( "ZPLIB", zplib, MOVE ) ;
	vcopy( "ZTLIB", ztlib, MOVE ) ;
	vcopy( "ZORXPATH", zorxpath, MOVE ) ;
	vcopy( "ZLDPATH", zldpath, MOVE ) ;

	sel = "" ;

	libs = "" ;
	qlibdef( "ZMUSR", "", "LIBS" ) ;
	pvar = "ZMUSR" ;
	desc = "LIBDEF user path for messages" ;
	for ( i = 1 ; i <= getpaths( libs ) ; i++ )
	{
		message = "" ;
		path    = getpath( libs, i ) ;
		if ( !is_directory( path ) ) { message = "Path not found" ; }
		tbadd( table ) ;
		desc = "" ;
		pvar = "" ;
	}

	libs = "" ;
	qlibdef( "ZMLIB", "", "LIBS" ) ;
	pvar = "ZMLIB" ;
	desc = "LIBDEF application path for messages" ;
	for ( i = 1 ; i <= getpaths( libs ) ; i++ )
	{
		message = "" ;
		path    = getpath( libs, i ) ;
		if ( !is_directory( path ) ) { message = "Path not found" ; }
		tbadd( table ) ;
		desc = "" ;
		pvar = "" ;
	}

	libs = "" ;
	qlibdef( "ZPUSR", "", "LIBS" ) ;
	sel  = "" ;
	pvar = "ZPUSR" ;
	desc = "LIBDEF user path for panels" ;
	for ( i = 1 ; i <= getpaths( libs ) ; i++ )
	{
		message = "" ;
		path    = getpath( libs, i ) ;
		if ( !is_directory( path ) ) { message = "Path not found" ; }
		tbadd( table ) ;
		desc = "" ;
		pvar = "" ;
	}

	libs = "" ;
	qlibdef( "ZPLIB", "", "LIBS" ) ;
	pvar = "ZPLIB" ;
	desc = "LIBDEF application path for panels" ;
	for ( i = 1 ; i <= getpaths( libs ) ; i++ )
	{
		message = "" ;
		path    = getpath( libs, i ) ;
		if ( !is_directory( path ) ) { message = "Path not found" ; }
		tbadd( table ) ;
		desc = "" ;
		pvar = "" ;
	}

	libs = "" ;
	qlibdef( "ZTUSR", "", "LIBS" ) ;
	sel  = "" ;
	pvar = "ZTUSR" ;
	desc = "LIBDEF user path for tables" ;
	for ( i = 1 ; i <= getpaths( libs ) ; i++ )
	{
		message = "" ;
		path    = getpath( libs, i ) ;
		if ( !is_directory( path ) ) { message = "Path not found" ; }
		tbadd( table ) ;
		desc = "" ;
		pvar = "" ;
	}

	libs = "" ;
	qlibdef( "ZTLIB", "", "LIBS" ) ;
	pvar = "ZTLIB" ;
	desc = "LIBDEF application path for tables" ;
	for ( i = 1 ; i <= getpaths( libs ) ; i++ )
	{
		message = "" ;
		path    = getpath( libs, i ) ;
		if ( !is_directory( path ) ) { message = "Path not found" ; }
		tbadd( table ) ;
		desc = "" ;
		pvar = "" ;
	}

	sel  = ""        ;
	pvar = "ZLDPATH" ;
	desc = "Path for application modules" ;
	for ( i = 1 ; i <= getpaths( zldpath ) ; i++)
	{
		path = getpath( zldpath, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc = "" ;
		pvar = "" ;
	}

	pvar = "ZMLIB" ;
	desc = "System search path for messages" ;
	for ( i = 1 ; i <= getpaths( zmlib ) ; i++)
	{
		path = getpath( zmlib, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc = "" ;
		pvar = "" ;
	}

	pvar = "ZPLIB" ;
	desc = "System search path for panels" ;
	for ( i = 1 ; i <= getpaths( zplib ) ; i++)
	{
		path = getpath( zplib, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc = "" ;
		pvar = "" ;
	}

	pvar = "ZTLIB" ;
	desc = "System search path for tables" ;
	for ( i = 1 ; i <= getpaths( ztlib ) ; i++)
	{
		path = getpath( ztlib, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) message = "Path not found" ;
		tbadd( table ) ;
		desc = "" ;
		pvar = "" ;
	}

	pvar = "ZTABL" ;
	vget( "ZTABL", PROFILE ) ;
	vcopy( "ZTABL", path, MOVE ) ;
	desc = "Table output directory" ;
	tbadd( table )    ;

	pvar = "ZSYSPATH" ;
	vget( "ZSYSPATH", PROFILE ) ;
	vcopy( "ZSYSPATH", path, MOVE ) ;
	desc = "System Path" ;
	tbadd( table )    ;

	pvar = "ZUPROF" ;
	vget( "ZUPROF", PROFILE ) ;
	vcopy( "ZUPROF", path, MOVE ) ;
	desc = "User home profile directory" ;
	tbadd( table )    ;

	pvar = "ZORXPATH" ;
	desc = "Object REXX EXEC search path" ;
	for ( i = 1 ; i <= getpaths( zorxpath ) ; i++)
	{
		path = getpath( zorxpath, i ) ;
		message = "" ;
		if ( !is_directory( path ) ) { message = "Path not found" ; }
		tbadd( table ) ;
		desc = "" ;
		pvar = "" ;
	}

	tbtop( table ) ;
	msg    = "" ;
	ztdtop = 1  ;
	e_loop = false ;
	while ( true )
	{
		tbtop( table ) ;
		tbskip( table, ztdtop ) ;
		if ( msg == "" ) { zcmd  = "" ; }
		tbdispl( table, "PPSP01AP", msg, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		msg = ""     ;
		if ( ztdsels == 0 && zcurinx != 0 )
		{
			tbtop( table ) ;
			tbskip( table, zcurinx ) ;
			ztdsels = 1   ;
			sel     = "S" ;
		}
		while ( ztdsels > 0 )
		{
			if ( sel == "S" )
			{
				if ( is_directory( path ) )
				{
					message = "*Listed*" ;
					vcopy( "ZFLSTPGM", pgm, MOVE ) ;
					select( "PGM(" + pgm + ") PARM(" + path + ")" ) ;
				}
				else
				{
					message = "*Error*" ;
				}
			}
			sel = ""       ;
			tbput( table ) ;
			if ( ztdsels > 1 )
			{
				tbdispl( table ) ;
				if ( RC > 4 ) { e_loop = true ; break ; }
			}
			else { ztdsels = 0 ; }
		}
		if ( e_loop ) { break ; }
	}
	tbend( table ) ;
}


void PPSP01A::libdefStatus()
{
	size_t p ;

	bool stacked ;

	string table ;
	string msg   ;
	string stk   ;
	string libx  ;
	string type  ;
	string usr   ;
	string ident ;
	string musr  ;
	string pusr  ;
	string tusr  ;
	string tabu  ;

	vdefine( "LDSTK LDLIB LDTYP LDUSR LDID", &stk, &libx, &type, &usr, &ident ) ;
	vdefine( "ZMUSR ZPUSR ZTUSR ZTABU", &musr, &pusr, &tusr, &tabu ) ;

	table = "LIBDFS" + d2ds( taskid(), 2 ) ;

	tbcreate( table, "", "(LDSTK,LDLIB,LDTYP,LDUSR,LDID)", NOWRITE ) ;

	map<string,stack<string>> zlibd = get_zlibd() ;

	map<int, stack<string>*> libdefs ;
	map<int, string>   lib ;
	map<int, string*> ulib ;

	libdefs[ 0 ] = &zlibd[ "ZMLIB" ] ;
	libdefs[ 1 ] = &zlibd[ "ZPLIB" ] ;
	libdefs[ 2 ] = &zlibd[ "ZTLIB" ] ;
	libdefs[ 3 ] = &zlibd[ "ZTABL" ] ;

	lib[ 0 ] = "ZMLIB" ;
	lib[ 1 ] = "ZPLIB" ;
	lib[ 2 ] = "ZTLIB" ;
	lib[ 3 ] = "ZTABL" ;

	ulib[ 0 ] = &musr ;
	ulib[ 1 ] = &pusr ;
	ulib[ 2 ] = &tusr ;
	ulib[ 3 ] = &tabu ;

	vget( "ZMUSR ZPUSR ZTUSR ZTABU", PROFILE ) ;

	for ( int l = 0 ; l < libdefs.size() ; l++ )
	{
		stack<string>* libdef = libdefs[ l ] ;
		if ( libdef->empty() )
		{
			stk   = "" ;
			libx  = lib[ l ] ;
			type  = "" ;
			usr   = "" ;
			ident = "** LIBDEF not active **" ;
			tbadd( table ) ;
			continue ;
		}
		libx = lib[ l ] ;
		if ( !libdef->empty() )
		{
			usr  = "X"    ;
			type = "FILE" ;
			stk  = ""     ;
			p    = getpaths( *ulib[ l ] ) ;
			for ( size_t i = 1 ; i <= p ; i++ )
			{
				ident = getpath( *ulib[ l ], i ) ;
				tbadd( table ) ;
				libx = "" ;
				type = "" ;
				libx = "" ;
			}
		}
		stacked = false ;
		while ( !libdef->empty() )
		{
			usr  = ""     ;
			type = "FILE" ;
			p    = getpaths( libdef->top() ) ;
			for ( size_t i = 1 ; i <= p ; i++ )
			{
				ident = getpath( libdef->top(), i ) ;
				stk   = (stacked && type != "" ) ? "S" : "" ;
				tbadd( table ) ;
				libx = "" ;
				type = "" ;
			}
			stacked = true ;
			libx = lib[ l ] ;
			libdef->pop() ;
		}
	}

	ztdtop = 1 ;
	addpop( "", 5, 5 ) ;

	while ( true )
	{
		tbtop( table ) ;
		tbskip( table, ztdtop ) ;
		if ( msg == "" ) { zcmd  = "" ; }
		tbdispl( table, "PPSP01LD", msg, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
	}

	rempop() ;
	tbend( table ) ;
	vdelete( "LDSTK LDLIB LDTYP LDUSR LDID" ) ;
	vdelete( "ZMUSR ZPUSR ZTUSR ZTABU" ) ;
}


void PPSP01A::showCommandTables()
{
	string cmdtab   ;
	string ocmdtab  ;
	string zctverb  ;
	string zcttrunc ;
	string zctact   ;
	string zctdesc  ;
	string msg      ;
	string applcmd  ;
	string applcmdl ;
	string panel    ;

	vdefine( "ZCTVERB ZCTTRUNC ZCTACT ZCTDESC", &zctverb, &zcttrunc, &zctact, &zctdesc ) ;
	vdefine( "ZAPPLID CMDTAB APPLCMD APPLCMDL ZVERB", &zapplid, &cmdtab, &applcmd, &applcmdl, &zverb ) ;

	vget( "ZAPPLID" ) ;
	vget( "CMDTAB", PROFILE ) ;
	if ( cmdtab == "" ) { cmdtab = "ISP" ; }
	ocmdtab = cmdtab ;

	applcmd  = "" ;
	applcmdl = "" ;
	if ( zapplid != "ISP" )
	{
		applcmd = zapplid ;
		tbopen( applcmd+"CMDS", NOWRITE, "", SHARE ) ;
		if ( RC > 4 )
		{
			applcmdl = "Application Command Table Not Found" ;
		}
		else
		{
			applcmdl = "" ;
			tbend( applcmd+"CMDS" ) ;
		}
	}
	msg = "" ;

	tbopen( cmdtab+"CMDS", NOWRITE, "", SHARE ) ;
	if ( RC > 0 )
	{
		cmdtab = "ISP" ;
		tbopen( cmdtab+"CMDS", NOWRITE, "", SHARE ) ;
	}
	ztdtop = 1 ;
	panel = "PPSP01AC" ;
	control( "PASSTHRU", "LRSCROLL", "PASON" ) ;
	while ( true )
	{
		tbtop( cmdtab+"CMDS" ) ;
		tbskip( cmdtab+"CMDS", ztdtop ) ;
		zcmd  = "" ;
		tbdispl( cmdtab+"CMDS", panel, msg, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		if ( ocmdtab != cmdtab )
		{
			tbopen( cmdtab+"CMDS", NOWRITE, "", SHARE ) ;
			if ( RC == 0 )
			{
				tbend( ocmdtab+"CMDS" ) ;
				ocmdtab = cmdtab ;
			}
			else
			{
				cmdtab = ocmdtab ;
				msg = "PPSP014"  ;
			}
		}
		vget( "ZVERB", SHARED ) ;
		if ( zverb == "LEFT" || zverb == "RIGHT" )
		{
			panel == "PPSP01AC" ? panel = "PPSP01AD" : panel = "PPSP01AC" ;
		}
	}
	tbend( cmdtab+"CMDS" ) ;
	vput( "CMDTAB", PROFILE ) ;

}


void PPSP01A::showLoadedClasses()
{
	uint j ;

	bool ref    ;
	bool e_loop ;

	string w1  ;
	string w2  ;
	string w3  ;
	string msg ;
	string psort ;

	string modlst  ;
	string appl    ;
	string mod     ;
	string modpath ;
	string status  ;

	lspfCommand lc ;

	vdefine( "SEL APPL MOD MODPATH STATUS", &sel, &appl, &mod, &modpath, &status ) ;

	modlst = "MODLST" + d2ds( taskid(), 2 ) ;

	msg    = ""    ;
	ztdtop = 1     ;
	ref    = true  ;
	e_loop = false ;
	psort  = "(APPL,C,A)" ;
	while ( true )
	{
		if ( ref )
		{
			tbcreate( modlst, "APPL", "(SEL,MOD,MODPATH,STATUS)", NOWRITE ) ;
			tbsort( modlst, psort ) ;
			lc.Command = "MODULE STATUS" ;
			lspfCallback( lc ) ;
			for ( j = 0 ; j < lc.reply.size() ; j++ )
			{
				sel     = ""              ;
				appl    = lc.reply[   j ] ;
				mod     = lc.reply[ ++j ] ;
				modpath = lc.reply[ ++j ] ;
				modpath = substr( modpath, 1, (lastpos( "/", modpath ) - 1) ) ;
				status  = lc.reply[ ++j ] ;
				tbadd( modlst, "", "ORDER" ) ;
			}
			ref = false ;
		}
		tbtop( modlst ) ;
		tbskip( modlst, ztdtop ) ;
		if ( msg == "" ) { zcmd = "" ; }
		tbdispl( modlst, "PPSP01ML", msg, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		if ( ztdsels == 0 && zcmd == "" ) { ref = true ; }
		w1 = word( zcmd, 1 ) ;
		w2 = word( zcmd, 2 ) ;
		w3 = word( zcmd, 3 ) ;
		if ( w1 == "SORT" )
		{
			if ( w2 == "" ) { w2 = "APPL" ; }
			if ( w3 == "" ) { w3 = "A"    ; }
			if      ( abbrev( "MODULES", w2, 3 ) )      { psort = "MOD,C,"+w3     ; }
			else if ( abbrev( "APPLICATIONS", w2, 3 ) ) { psort = "APPL,C,"+w3    ; }
			else if ( abbrev( "PATHS", w2, 3 ) )        { psort = "MODPATH,C,"+w3 ; }
			else if ( abbrev( "STATUS", w2, 3 ) )       { psort = "STATUS,C,"+w3  ; }
			else                                        { msg   = "PSYS011C" ; continue ; }
			tbsort( modlst, psort ) ;
			continue ;
		}
		while ( ztdsels > 0 )
		{
			if ( sel == "R" )
			{
				lc.Command = "MODREL " + appl ;
				lspfCallback( lc ) ;
			}
			if ( ztdsels > 1 )
			{
				tbdispl( modlst ) ;
				if ( RC > 4 ) { e_loop = true ; break ; }
			}
			else { ztdsels = 0 ; }
		}
		if ( e_loop ) { break ; }
		if ( ref ) { tbend( modlst ) ; }
	}
	tbend( modlst ) ;
	return ;
}


void PPSP01A::showSavedFileList()
{
	int i ;
	string zfile ;
	string zfiln ;
	string zcurr ;
	string zdir  ;
	string pgm   ;
	string msg   ;

	vdefine( "ZCURR ZFILE ZDIR", &zcurr, &zfile, &zdir ) ;

	msg = "" ;
	while ( true )
	{
		display( "PPSP01FL", msg, "ZCMD" ) ;
		if ( RC == 8 ) { return ; }

		if ( zfile != "" )
		{
			if ( zfile == "*" ) { zfile = "" ; }
			if ( zfile != "" && zfile[ 0 ] == '/' )
			{
				 zfiln = zfile ;
			}
			else if ( zdir != "" )
			{
				zfiln = zdir + "/" + zfile ;
			}
			else if ( zcurr != "" )
			{
				zfiln = zcurr + "/" + zfile ;
			}
			else { continue ; }
			if ( is_directory( zfiln ) )
			{
				vcopy( "ZFLSTPGM", pgm, MOVE ) ;
				select( "PGM(" + pgm + ") PARM(" + zfiln + ")" ) ;
			}
			else if ( is_regular_file( zfiln ) )
			{
				browse( zfiln ) ;
			}
			continue ;
		}
		for ( i = 1 ; i < 9 ; i++ )
		{
			vcopy( "SEL" + d2ds(i), sel, MOVE ) ;
			if ( sel == "" || RC == 8 ) { continue ; }
			vreplace( "SEL" + d2ds(i), "" ) ;
			vcopy( "ZFILE" + d2ds(i), zfiln, MOVE ) ;
			if ( zfiln == "" || RC == 8 ) { continue ; }
			if ( sel == "S" || sel == "L" )
			{
				if ( is_directory( zfiln ) )
				{
					vcopy( "ZFLSTPGM", pgm, MOVE ) ;
					select( "PGM(" + pgm + ") PARM(" + zfiln + ")" ) ;
				}
			}
			else if ( sel == "B" )
			{
				if ( is_regular_file( zfiln ) )
				{
					browse( zfiln ) ;
				}
			}
			else if ( sel == "E" )
			{
				if ( is_regular_file( zfiln ) )
				{
					edit( zfiln ) ;
				}
			}
		}
	}
}


void PPSP01A::showTasks()
{
	int retc ;

	bool e_loop  ;

	string of    ;
	string uf    ;
	string zcmd  ;
	string user  ;
	string pid   ;
	string cpu   ;
	string cpux  ;
	string mem   ;
	string memx  ;
	string cmd   ;
	string msg   ;
	string table ;

	msg = "" ;

	vdefine( "SEL USER PID CPU CPUX MEM MEMX CMD", &sel, &user, &pid, &cpu, &cpux, &mem, &memx, &cmd ) ;

	table = "TSKLST" + d2ds( taskid(), 2 ) ;

	updateTasks( table ) ;

	ztdtop = 1 ;
	e_loop = false ;
	while ( true )
	{
		tbtop( table ) ;
		tbskip( table, ztdtop ) ;
		zcmd  = "" ;
		tbdispl( table, "PPSP01TK", msg, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		vcopy( "USERF", uf, MOVE ) ;
		vcopy( "ONLYF", of, MOVE ) ;
		while ( ztdsels > 0 )
		{
			if ( sel == "K")
			{
				retc = kill( ds2d( pid ), SIGKILL ) ;
				llog( "I", "Kill signal sent to PID " << pid << ".  RC=" << retc << endl ) ;
			}
			if ( sel == "T")
			{
				retc = kill( ds2d( pid ), SIGTERM ) ;
				llog( "I", "Terminate signal sent to PID " << pid << ".  RC=" << retc << endl ) ;
			}
			else if ( sel == "S")
			{
				select( "PGM(PCMD0A) PARM(systemctl status "+ pid +" )" ) ;
			}
			if ( ztdsels > 1 )
			{
				tbdispl( table ) ;
				if ( RC > 4 ) { e_loop = true ; break ; }
			}
			else { ztdsels = 0 ; }
		}
		if ( e_loop ) { break ; }
		tbend( table ) ;
		updateTasks( table, uf, of ) ;
	}
	tbend( table ) ;
	vdelete( "SEL USER PID CPU CPUX MEM MEMX CMD" ) ;
}


void PPSP01A::updateTasks( const string& table, const string& uf, const string& of )
{
	// Columns for top: PID, user, priority, NI, virt, RES, Size, Status, %CPU, %MEM, time, Command

	// Procedure can use 'top' or 'ps'
	// top gives a better representation of CPU percentage but there is a delay

	size_t p ;

	string inLine ;
	string pid    ;
	string cmd    ;
	string user   ;
	string cpu    ;
	string cpux   ;
	string tname  ;
	string cname  ;

	enum Columns
	{
		C_PID = 1,
		C_USER,
		C_PRI,
		C_NI,
		C_VIRT,
		C_RES,
		C_SIZE,
		C_STATUS,
		C_CPU,
		C_MEM,
		C_TIME,
		C_CMD
	} ;

	std::ifstream fin ;

	vcopy( "ZUSER", zuser, MOVE )     ;
	vcopy( "ZSCREEN", zscreen, MOVE ) ;

	boost::filesystem::path temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path( zuser + "-" + zscreen + "-%%%%-%%%%" ) ;
	tname = temp.native() ;

  //    cname = "ps ax -o pid,user,pri,ni,vsz,drs,size,stat,%cpu,%mem,time,cmd > " + tname ;
	cname = "top -b -n 1 > " + tname ;

	tbcreate( table, "", "(SEL,USER,PID,CPU,CPUX,MEM,MEMX,CMD)", NOWRITE ) ;

	system( cname.c_str() ) ;
	fin.open( tname ) ;

	while ( getline( fin, inLine ) )
	{
		tbvclear( table ) ;
		pid = word( inLine, C_PID ) ;
		if ( !datatype( pid, 'W' ) ) { continue ; }
		vreplace( "PID", pid )   ;
		user = word( inLine, C_USER ) ;
		if ( uf != "" && uf != upper( user ) ) { continue ; }
		vreplace( "USER", user ) ;
		cpu  = word( inLine, C_CPU ) ;
		vreplace( "CPU", cpu ) ;
		p = cpu.find( '.' )      ;
		if ( p == string::npos ) { cpux = d2ds( ds2d( cpu ) * 10 ) ; }
		else                     { cpux = cpu ; cpux.erase( p, 1 ) ; }
		vreplace( "CPUX", cpux ) ;
		vreplace( "MEM", word( inLine, C_MEM ) ) ;
		cmd = subword( inLine, C_CMD ) ;
		if ( of != "" && pos( of, upper( cmd ) ) == 0 ) { continue ; }
		trim_left( cmd ) ;
		cmd = strip( cmd, 'L', '`' ) ;
		cmd = strip( cmd, 'L', '-' ) ;
		trim_left( cmd ) ;
		vreplace( "CMD", cmd ) ;
		tbadd( table ) ;
	}
	fin.close() ;
	tbsort( table, "(CPUX,N,D)" ) ;
	tbtop( table ) ;
	remove( tname ) ;
}


void PPSP01A::utilityPrograms()
{

	int RCode ;

	string kmainpgm ;
	string kmainpan ;
	string kpanlpgm ;
	string keditpgm ;
	string kbrpgm   ;
	string kviewpgm ;
	string kflstpgm ;
	string khelppgm ;
	string korexpgm ;

	string v_list1 = "ZMAINPGM ZMAINPAN ZPANLPGM ZEDITPGM ZBRPGM ZVIEWPGM ZFLSTPGM ZHELPPGM" ;
	string v_list2 = "ZOREXPGM" ;

	vdefine( v_list1, &kmainpgm, &kmainpan, &kpanlpgm, &keditpgm, &kbrpgm, &kviewpgm, &kflstpgm, &khelppgm ) ;
	vget( v_list1, PROFILE ) ;

	vdefine( v_list2, &korexpgm ) ;
	vget( v_list2, PROFILE ) ;

	while ( true )
	{
		zcmd = "" ;
		display( "PPSP01UP", "", "ZCMD" ) ;
		RCode = RC ;

		if ( kmainpgm == "" ) { kmainpgm = ZMAINPGM ; } ;
		if ( kmainpan == "" ) { kmainpan = ZMAINPAN ; } ;
		if ( kpanlpgm == "" ) { kpanlpgm = ZPANLPGM ; } ;
		if ( keditpgm == "" ) { keditpgm = ZEDITPGM ; } ;
		if ( kbrpgm   == "" ) { kbrpgm   = ZBRPGM   ; } ;
		if ( kviewpgm == "" ) { kviewpgm = ZVIEWPGM ; } ;
		if ( kflstpgm == "" ) { kflstpgm = ZFLSTPGM ; } ;
		if ( khelppgm == "" ) { khelppgm = ZHELPPGM ; } ;
		if ( korexpgm == "" ) { korexpgm = ZOREXPGM ; } ;

		if ( zcmd == "CANCEL" ) { break ; }
		if ( zcmd == "DEFAULTS" )
		{
			kmainpgm = ZMAINPGM ;
			kmainpan = ZMAINPAN ;
			kpanlpgm = ZPANLPGM ;
			keditpgm = ZEDITPGM ;
			kbrpgm   = ZBRPGM   ;
			kviewpgm = ZVIEWPGM ;
			kflstpgm = ZFLSTPGM ;
			khelppgm = ZHELPPGM ;
			korexpgm = ZOREXPGM ;
		}

		if ( RCode == 8 || zcmd == "SAVE" )
		{
			vput( v_list1, PROFILE ) ;
			vput( v_list2, PROFILE ) ;
			if ( RCode == 8 ) { break ; }
		}
	}
	vdelete( v_list1 ) ;
	vdelete( v_list2 ) ;
}

void PPSP01A::keylistTables()
{
	// Show a list of all key list tables in the ZUPROF path
	// If there are no tables found, create an empty ISPKEYP

	int RC1 ;

	bool e_loop  ;

	string msg   ;
	string cw    ;
	string w2    ;
	string tab   ;
	string fname ;
	string p     ;

	string tbk1sel ;
	string tbk1tab ;
	string tbk1typ ;
	string tbk1msg ;
	string table   ;
	string uprof   ;
	string newtab  ;

	string aktab   ;
	string aklist  ;

	typedef vector<path> vec ;
	vec v ;

	vec::const_iterator it ;

	vdefine( "TBK1SEL TBK1TAB TBK1TYP TBK1MSG NEWTAB", &tbk1sel, &tbk1tab, &tbk1typ, &tbk1msg, &newtab ) ;
	table = "KEYP" + d2ds( taskid(), 4 ) ;

	tbcreate( table, "", "(TBK1SEL,TBK1TAB,TBK1TYP,TBK1MSG)", NOWRITE ) ;
	if ( RC > 0 ) { abend() ; }

	tbsort( table, "(TBK1TAB,C,A)" ) ;

	vcopy( "ZUPROF", uprof, MOVE ) ;
	vcopy( "ZKLNAME", aklist, MOVE ) ;
	vcopy( "ZKLAPPL", aktab, MOVE ) ;

	copy( directory_iterator( uprof ), directory_iterator(), back_inserter( v ) ) ;

	for ( it = v.begin() ; it != v.end() ; ++it )
	{
		fname = (*it).string() ;
		p     = substr( fname, 1, (lastpos( "/", fname ) - 1) ) ;
		tab   = substr( fname, (lastpos( "/", fname ) + 1) )    ;
		if ( tab.size() < 5 ) { continue ; }
		if ( tab.compare( tab.size()-4, 4, "KEYP" ) == 0 )
		{
			tbvclear( table ) ;
			tbk1tab = tab    ;
			if ( tbk1tab == aktab+"KEYP" )
			{
				tbk1msg = "*Active*" ;
			}
			tbk1typ = "Private" ;
			tbadd( table, "", "ORDER" ) ;
		}
	}

	tbtop( table )     ;
	tbskip( table, 1 ) ;
	if ( RC == 8 )
	{
		newtab = "ISP" ;
		createKeyTable( newtab ) ;
		tbvclear( table ) ;
		tbk1tab = newtab+"KEYP" ;
		tbadd( table )  ;
	}

	msg    = "" ;
	ztdtop = 1  ;
	e_loop = false ;
	while ( true )
	{
		tbtop( table ) ;
		tbskip( table, ztdtop ) ;
		if ( msg == "" ) { zcmd = "" ; }
		tbdispl( table, "PPSP01K1", msg, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		while ( ztdsels > 0 )
		{
			if ( tbk1sel == "D" )
			{
				addpop( "", 5, 5 ) ;
				display( "PPSP01K7", msg, "ZCMD" ) ;
				RC1 = RC ;
				rempop() ;
				if ( RC1 == 0 )
				{
					remove( uprof + "/" + tbk1tab ) ;
					tbdelete( table ) ;
					if ( RC > 0 ) { abend() ; }
				}
			}
			else if ( tbk1sel == "N" )
			{
				addpop( "", 5, 5 ) ;
				display( "PPSP01K5", msg, "ZCMD" ) ;
				if ( RC == 0 )
				{
					tbk1tab = newtab + "KEYP" ;
					tbk1msg = "*Added*" ;
					tbadd( table, "", "ORDER" ) ;
					createKeyTable( newtab ) ;
					tbk1sel = ""  ;
					tbput( table ) ;
				}
				rempop() ;
			}
			else if ( tbk1sel == "S" )
			{
				control( "DISPLAY", "SAVE" ) ;
				keylistTable( tbk1tab, aktab, aklist ) ;
				control( "DISPLAY", "RESTORE" ) ;
				tbk1msg = "*Selected*" ;
				tbk1sel = ""  ;
				tbput( table ) ;
			}
			if ( ztdsels > 1 )
			{
				tbdispl( table ) ;
				if ( RC > 4 ) { e_loop = true ; break ; }
			}
			else { ztdsels = 0 ; }
		}
		if ( e_loop ) { break ; }
	}
	vdelete( "TBK1SEL TBK1TAB TBK1TYP TBK1MSG NEWTAB" ) ;
	return ;
}


void PPSP01A::keylistTable( string tab, string aktab, string aklist )
{
	// Show keylist table (default current profile)
	// If there are no rows in table tab, create an empty ISPDEF entry

	int i ;
	int RC1 ;

	bool e_loop ;

	string msg  ;
	string cw   ;
	string w2   ;
	string fname;
	string p    ;
	string t    ;

	string tbk2sel  ;
	string tbk2lst  ;
	string tbk2msg  ;
	string keylistn ;
	string table    ;
	string uprof    ;
	string newkey   ;

	bool   actTab   ;

	if ( aktab == "" )
	{
		vcopy( "ZKLNAME", aklist, MOVE ) ;
		vcopy( "ZKLAPPL", aktab, MOVE ) ;
		actTab = true   ;
		if ( aktab == "" )
		{
			vcopy( "ZAPPLID", aktab, MOVE ) ;
			actTab = false ;
		}
	}
	else
	{
		actTab = ( aktab+"KEYP" == tab ) ;
	}

	if ( tab == "" )
	{
		tab = aktab+"KEYP" ;
		vreplace( "TBK1TAB", tab ) ;
	}

	vdefine( "TBK2SEL TBK2LST TBK2MSG KEYLISTN NEWKEY", &tbk2sel, &tbk2lst, &tbk2msg, &keylistn, &newkey ) ;
	table = "KLT2" + d2ds( taskid(), 4 ) ;
	vcopy( "ZUPROF", uprof, MOVE ) ;

	tbopen( tab, NOWRITE, uprof ) ;
	if ( RC > 0 )
	{
		llog( "E", "Error opening Keylist table "<< tab << ".  RC="<< RC << endl ) ;
		abend() ;
	}

	tbcreate( table, "", "(TBK2SEL,TBK2LST,TBK2MSG)", NOWRITE ) ;
	if ( RC > 0 )
	{
		llog( "E", "Error creating Keylist table "<< table << ".  RC="<< RC << endl ) ;
		abend() ;
	}

	tbtop( tab ) ;
	tbskip( tab, 1 ) ;
	if ( RC > 0 )
	{
		tbend( tab ) ;
		tbopen( tab, WRITE, uprof ) ;
		if ( RC > 0 ) { abend() ; }
		tbsort( tab, "(KEYLISTN,C,A)" ) ;
		keylistn = "ISPDEF" ;
		for ( i = 1 ; i < 25 ; i++ )
		{
			vreplace( "KEY"+d2ds(i)+"DEF", "" ) ;
			vreplace( "KEY"+d2ds(i)+"ATR", "" ) ;
			vreplace( "KEY"+d2ds(i)+"LAB", "" ) ;
		}
		vreplace( "KEYHELPN", "" ) ;
		tbadd( tab, "", "ORDER" ) ;
		if ( RC > 0 ) { abend() ; }
		tbclose( tab, "", uprof ) ;
		tbopen( tab, NOWRITE, uprof ) ;
		if ( RC > 0 ) { abend() ; }
	}

	tbtop( tab ) ;
	while ( true )
	{
		tbskip( tab ) ;
		if ( RC > 0 ) { break ; }
		tbvclear( table ) ;
		tbk2lst = keylistn ;
		if ( actTab && tbk2lst == aklist )
		{
			tbk2msg = "*Active*" ;
		}
		tbadd( table ) ;
		if ( RC > 0 ) { abend() ; }
	}
	tbend( tab ) ;

	tbsort( table, "(TBK2LST,C,A)" ) ;

	ztdtop = 1 ;
	e_loop = false ;
	while ( true )
	{
		tbtop( table ) ;
		tbskip( table, ztdtop ) ;
		if ( msg == "" ) { zcmd = "" ; }
		tbdispl( table, "PPSP01K2", msg, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
		while ( ztdsels > 0 )
		{
			if ( tbk2sel == "D" )
			{
				addpop( "", 5, 5 ) ;
				display( "PPSP01K7", msg, "ZCMD" ) ;
				RC1 = RC ;
				rempop() ;
				if ( RC1 == 0 )
				{
					tbopen( tab, WRITE, uprof ) ;
					if ( RC > 0 ) { abend() ; }
					keylistn = tbk2lst ;
					tbdelete( tab )   ;
					if ( RC > 0 ) { abend() ; }
					tbdelete( table ) ;
					if ( RC > 0 ) { abend() ; }
					tbclose( tab, "", uprof ) ;
				}
			}
			else if ( tbk2sel == "N" )
			{
				addpop( "", 5, 5 ) ;
				display( "PPSP01K4", msg, "ZCMD" ) ;
				RC1 = RC ;
				rempop() ;
				if ( RC1 == 0 )
				{
					tbopen( tab, NOWRITE, uprof ) ;
					if ( RC > 0 ) { abend() ; }
					tbvclear( tab ) ;
					keylistn = newkey ;
					tbget( tab ) ;
					if ( RC == 0 )
					{
						tbend( tab ) ;
						tbk2msg = "*Exists*" ;
					}
					else
					{
						tbend( tab ) ;
						tbopen( tab, WRITE, uprof ) ;
						if ( RC > 0 ) { abend() ; }
						tbsort( tab, "(KEYLISTN,C,A)" ) ;
						keylistn = newkey ;
						for ( i = 1 ; i < 25 ; i++ )
						{
							vreplace( "KEY"+d2ds(i)+"DEF", "" ) ;
							vreplace( "KEY"+d2ds(i)+"ATR", "" ) ;
							vreplace( "KEY"+d2ds(i)+"LAB", "" ) ;
						}
						vcopy( "KEYHELP", t, MOVE ) ;
						vreplace( "KEYHELPN", t ) ;
						tbadd( tab, "", "ORDER" ) ;
						if ( RC > 0 ) { abend() ; }
						tbclose( tab, "", uprof ) ;
						tbk2lst = newkey ;
						tbk2msg = "*Added*" ;
						tbadd( table, "", "ORDER" ) ;
						if ( RC > 0 ) { abend() ; }
						control( "DISPLAY", "SAVE" ) ;
						editKeylist( tab, newkey ) ;
						control( "DISPLAY", "RESTORE" ) ;
					}
					tbk2sel = ""  ;
					tbput( table ) ;
				}
			}
			else if ( tbk2sel == "E" )
			{
				control( "DISPLAY", "SAVE" ) ;
				editKeylist( tab, tbk2lst ) ;
				control( "DISPLAY", "RESTORE" ) ;
				tbk2msg = "*Edited*" ;
				tbk2sel = ""  ;
				tbput( table ) ;
			}
			else if ( tbk2sel == "V" )
			{
				control( "DISPLAY", "SAVE" ) ;
				viewKeylist( tab, tbk2lst ) ;
				control( "DISPLAY", "RESTORE" ) ;
				tbk2msg = "*Viewed*" ;
				tbk2sel = ""  ;
				tbput( table ) ;
			}
			if ( ztdsels > 1 )
			{
				tbdispl( table ) ;
				if ( RC > 4 ) { e_loop = true ; break ; }
			}
			else { ztdsels = 0 ; }
		}
		if ( e_loop ) { break ; }
	}
	tbend( table ) ;
	vdelete( "TBK2SEL TBK2LST TBK2MSG KEYLISTN NEWKEY" ) ;
}


void PPSP01A::viewKeylist( const string& tab, const string& list )
{
	// Field names: KEYLISTN KEYnDEF KEYnLAB KEYnATR (n=1 to 24)
	// TD Field names: KEYNUM KEYDEF KEYATTR KEYLAB

	// Read keylist from table tab, KEYLISTN list and create table display.

	int i ;

	string t        ;
	string keynum   ;
	string keydef   ;
	string keyattr  ;
	string keylab   ;

	string keylistn ;
	string table    ;
	string uprof    ;
	string msg      ;

	table = "KLT4" + d2ds( taskid(), 4 ) ;

	vcopy( "ZUPROF", uprof, MOVE ) ;

	tbopen( tab, NOWRITE, uprof ) ;
	if ( RC > 0 ) { abend() ; }

	vdefine( "KEYLISTN KEYNUM KEYDEF KEYATTR KEYLAB", &keylistn, &keynum, &keydef, &keyattr, &keylab ) ;

	tbcreate( table, "KEYNUM", "(KEYDEF,KEYATTR,KEYLAB)", NOWRITE ) ;
	if ( RC > 0 ) { abend() ; }

	tbvclear( tab ) ;
	keylistn = list ;
	tbget( tab ) ;
	if ( RC > 0 ) { abend() ; }

	vcopy( "KEYHELPN", t, MOVE ) ;
	vreplace( "KEYHELP", t ) ;
	for ( i = 1 ; i < 25 ; i++ )
	{
		keynum = "F"+left( d2ds( i ), 2 ) + ". . ." ;
		vcopy( "KEY"+d2ds(i)+"DEF", keydef, MOVE ) ;
		vcopy( "KEY"+d2ds(i)+"ATR", keyattr, MOVE ) ;
		vcopy( "KEY"+d2ds(i)+"LAB", keylab, MOVE ) ;
		tbadd( table ) ;
		if ( RC > 0 ) { abend() ; }
	}
	tbend( tab ) ;

	ztdtop = 1 ;
	while ( true )
	{
		tbtop( table ) ;
		tbskip( table, ztdtop ) ;
		if ( msg == "" ) { zcmd = "" ; }
		tbdispl( table, "PPSP01K6", msg, "ZCMD" ) ;
		if ( RC == 8 ) { break ; }
		msg = "" ;
	}
	tbend( table )  ;
	vdelete( "KEYLISTN KEYNUM KEYDEF KEYATTR KEYLAB" ) ;
}


void PPSP01A::editKeylist( const string& tab, const string& list )
{
	// Field names: KEYLISTN KEYnDEF KEYnLAB KEYnATR (n=1 to 24)
	// TD Field names: KEYNUM KEYDEF KEYATTR KEYLAB

	// Read keylist from table tab, KEYLISTN list and create table display.
	// Update tab/list from table display.

	int i ;

	bool e_loop     ;

	string t        ;
	string keynum   ;
	string keydef   ;
	string keyattr  ;
	string keylab   ;

	string keylistn ;
	string table    ;
	string uprof    ;
	string msg      ;

	table = "KLT3" + d2ds( taskid(), 4 ) ;

	vcopy( "ZUPROF", uprof, MOVE ) ;

	tbopen( tab, NOWRITE, uprof ) ;
	if ( RC > 0 ) { abend() ; }

	vdefine( "KEYLISTN KEYNUM KEYDEF KEYATTR KEYLAB", &keylistn, &keynum, &keydef, &keyattr, &keylab ) ;

	tbcreate( table, "KEYNUM", "(KEYDEF,KEYATTR,KEYLAB)", NOWRITE ) ;
	if ( RC > 0 ) { abend() ; }

	tbvclear( tab ) ;
	keylistn = list ;
	tbget( tab ) ;
	if ( RC > 0 ) { abend() ; }

	vcopy( "KEYHELPN", t, MOVE ) ;
	vreplace( "KEYHELP", t ) ;
	for ( i = 1 ; i < 25 ; i++ )
	{
		keynum = "F"+left( d2ds( i ), 2 ) + ". . ." ;
		vcopy( "KEY"+d2ds(i)+"DEF", keydef, MOVE ) ;
		vcopy( "KEY"+d2ds(i)+"ATR", keyattr, MOVE ) ;
		vcopy( "KEY"+d2ds(i)+"LAB", keylab, MOVE ) ;
		tbadd( table ) ;
		if ( RC > 0 ) { abend() ; }
	}
	tbend( tab ) ;

	ztdtop = 1 ;
	e_loop = false ;
	while ( true )
	{
		tbtop( table ) ;
		tbskip( table, ztdtop ) ;
		if ( msg == "" ) { zcmd = "" ; }
		tbdispl( table, "PPSP01K3", msg, "ZCMD" ) ;
		if ( zcmd == "CANCEL" )
		{
			vdelete( "KEYLISTN KEYNUM KEYDEF KEYATTR KEYLAB" ) ;
			tbend( table ) ;
			if ( RC > 0 ) { abend() ; }
			return ;
		}
		if ( RC == 8 ) { break ; }
		msg = "" ;
		while ( ztdsels > 0 )
		{
			tbmod( table ) ;
			if ( ztdsels > 1 )
			{
				tbdispl( table ) ;
				if ( RC > 4 ) { e_loop = true ; break ; }
			}
			else { ztdsels = 0 ; }
		}
		if ( e_loop ) { break ; }
	}

	tbopen( tab, WRITE, uprof ) ;
	if ( RC > 0 ) { abend() ; }

	tbvclear( tab ) ;
	keylistn = list ;
	tbget( tab ) ;
	if ( RC > 0 ) { abend() ; }

	vcopy( "KEYHELP", t, MOVE ) ;
	vreplace( "KEYHELPN", t ) ;
	tbtop( table ) ;
	for ( i = 1 ; i < 25 ; i++ )
	{
		tbskip( table, 1 ) ;
		if ( RC > 0 ) { break ; }
		vreplace( "KEY"+d2ds(i)+"DEF", keydef ) ;
		vreplace( "KEY"+d2ds(i)+"ATR", keyattr ) ;
		vreplace( "KEY"+d2ds(i)+"LAB", keylab ) ;
	}

	tbmod( tab ) ;
	if ( RC > 0 ) { abend() ; }

	tbclose( tab, "", uprof ) ;
	if ( RC > 0 ) { abend() ; }

	tbend( table ) ;

	vdelete( "KEYLISTN KEYNUM KEYDEF KEYATTR KEYLAB" ) ;
}


void PPSP01A::createKeyTable( string table )
{
	// Create an empty keylist table entry.  Keylists reside in the ZUPROF directory.

	int i ;

	string uprof ;
	string flds  ;

	vcopy( "ZUPROF", uprof, MOVE ) ;
	table += "KEYP" ;
	flds = ""     ;

	for ( i = 1 ; i < 25 ; i++ )
	{
		flds += "KEY"+d2ds(i)+"DEF " ;
		flds += "KEY"+d2ds(i)+"ATR " ;
		flds += "KEY"+d2ds(i)+"LAB " ;
	}
	flds += "KEYHELPN" ;

	tbcreate( table, "KEYLISTN", "("+flds+")", WRITE, NOREPLACE, uprof ) ;
	if ( RC > 0 ) { abend() ; }

	tbsave( table, "", uprof ) ;
	if ( RC > 0 ) { abend() ; }

	tbend( table ) ;
}


void PPSP01A::runApplication( const string& xappl )
{
	select( "PGM("+xappl+") NEWAPPL(ISP) NEWPOOL PASSLIB" ) ;
}


// ============================================================================================ //

extern "C" { pApplication* maker() { return new PPSP01A ; } }
extern "C" { void destroy(pApplication* p) { delete p ; } }
