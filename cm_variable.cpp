//  cm_variable.cpp
//

#include "cm_variable.h"

#include <Shlobj.h>
#include <PathCch.h>

union cvalue { float f; int i; bool b; };

class cVar : public vVar
{
public:
    void    Create (char *szName, char *szValue, char *szType, int bitFlags, char *szDesc);
    void    Destroy ();

    void *operator new (size_t s) { return mem::alloc( s ); } 
    void operator delete (void *ptr) { mem::free( ptr ); }

    virtual void    setString (char *szString);
    virtual void    setFloat (float f);
    virtual void    setBool (bool b);
    virtual void    setInt (int i);

    virtual char    *getString ();
    virtual float   getFloat ();
    virtual bool    getBool ();
    virtual int     getInt ();

    virtual char        *getName () { return m_szName; }
    virtual char        *getDesc () { return m_szDesc; }
    virtual int         getFlags () { return m_bitFlags; }
    virtual cvar_type_t getType () { return m_Type; }

private:
    char    m_szName[SHORT_STRING];
    char    m_szValue[LONG_STRING];

    cvalue      m_Value;
    int         m_bitFlags;
    cvar_type_t m_Type;
    char    *m_szDesc;
};

/*=========================================================
=========================================================*/

typedef struct variable_link_s
{
    cVar            *pVariable;
    variable_link_s *pNext, *pPrev;
} var_link_t;

class cVariable : public vVariable
{
public:
    cVariable () { m_Head.pNext = m_Head.pPrev = &m_Head; Load( ); }
    ~cVariable () { Unload( ); }

    virtual int     Load ();
    virtual int     Unload ();

    virtual cvar_t  *Get (char *szName);
    virtual cvar_t  *Get (char *szName, char *szValue);
    virtual cvar_t  *Get (char *szName, char *szValue, char *szType, int bitFlags, char *szDesc);

    virtual int     List ();

private:
    cVar    *Create (char *szName, char *szValue, char *szType, int bitFlags, char *szDesc);
    cVar    *Find (char *szName);

    void    Delete (var_link_t *pLink);

    char    *Print (cVar *cvar, int tab_size = 8);

    var_link_t  m_Head;
};

DEF_CREATE_DESTROY(Variable);

/*=========================================================
=========================================================*/

int getConfigPath(char *path, int size, bool create = false)
{
    PWSTR pszPath;
    WCHAR wPath[1024];

    SHGetKnownFolderPath( FOLDERID_LocalAppData, KF_FLAG_DEFAULT, NULL, &pszPath );

    PathCchCombine(wPath, _countof(wPath), pszPath, L"Tanks!");

    if (create) {
        CreateDirectoryW(wPath, NULL);
    }

    PathCchAppend(wPath, _countof(wPath), L"config.ini" );

    CoTaskMemFree( pszPath );

    return WideCharToMultiByte(
        CP_ACP,
        WC_NO_BEST_FIT_CHARS,
        wPath,
        -1,
        path,
        size,
        NULL,
        NULL);
}

/*=========================================================
=========================================================*/

int cVariable::Load ()
{
    char szPath[LONG_STRING];

    char    *fileData, *fileCursor, getLine[MAX_STRING];
    int     nLength;

    textutils_c text;

    getConfigPath(szPath, _countof(szPath));

    file::load( (void **)&fileData, szPath, &nLength );

    if ( (fileCursor = fileData) == NULL )
        return ERROR_NONE;

    while ( *fileCursor )
    {
        fileCursor = text.getline( fileCursor, getLine, MAX_STRING );

        text.parse( getLine );

        if ( text.argc() < 2 )
            break;
        
        Create( text.argv(0), text.argv(1), text.argv(2), atoi(text.argv(3)), text.argv(4) );
    }

    file::unload( fileData );

    return ERROR_NONE;
}

/*=========================================================
=========================================================*/

int cVariable::Unload ()
{
    char szPath[LONG_STRING];

    FILE        *fOut;
    var_link_t  *pLink, *pNext;

    char        *szLine;

    getConfigPath(szPath, _countof(szPath), true);

    file::open( &fOut, szPath, "w+" );

    if ( !fOut )
        return ERROR_FAIL;

    for ( pLink = m_Head.pNext ; pLink != &m_Head ; pLink = pNext )
    {
        pNext = pLink->pNext;

        if ( pLink->pVariable->getFlags( ) & CVAR_ARCHIVE )
        {
            szLine = Print( pLink->pVariable );

            fwrite( (void *)szLine, 1, strlen(szLine), fOut );
        }

        Delete( pLink );
    }

    file::close( fOut );

    return ERROR_NONE;
}

/*=========================================================
=========================================================*/

char *cVariable::Print (cVar *cvar, int tab_size)
{
    static char szOut[MAX_STRING];

    memset( szOut, 0, MAX_STRING );
    fmt( szOut, "%-20s %-20s %-8s %3d \"%s\"\n",
         cvar->getName(),
         va("\"%s\"", cvar->getString()),
         cvar_type_strings[cvar->getType()],
         cvar->getFlags(),
         cvar->getDesc() );

    return szOut;
}

/*=========================================================
=========================================================*/

int cVariable::List ()
{
    var_link_t  *pLink;

    printf( "listing active variables...\n" );

    for ( pLink = m_Head.pNext ; pLink != &m_Head ; pLink = pLink->pNext )
        printf( Print( pLink->pVariable ) );

    return ERROR_NONE;
}

/*=========================================================
=========================================================*/

cvar_t *cVariable::Get (char *szName, char *szValue, char *szType, int bitFlags, char *szDesc)
{
    cVar    *cvar;

    cvar = Find( szName );

    if ( cvar )
    {
        cvar->Create( szName, cvar->getString( ), szType, bitFlags | cvar->getFlags( ), szDesc );
        return cvar;
    }

    return Create( szName, szValue, szType, bitFlags, szDesc );
}

cvar_t *cVariable::Get (char *szName, char *szValue)
{
    cVar    *cvar = Find( szName );

    if ( cvar )
        return cvar;

    return Create( szName, szValue, "undefined", 0, "" );
}

cvar_t *cVariable::Get (char *szName)
{
    return Find( szName );
}

/*=========================================================
=========================================================*/

cVar *cVariable::Find (char *szName)
{
    var_link_t  *link;
    int         cmp;

    for( link = m_Head.pNext ; link != &m_Head ; link = link->pNext )
    {
        cmp = stricmp( szName, link->pVariable->getName( ) );

        if ( cmp == 0 )
            return link->pVariable;
        else if ( cmp < 0 ) // passed it, does not exist
            return NULL;
    }

    return NULL;
}

/*=========================================================
=========================================================*/

cVar *cVariable::Create (char *szName, char *szValue, char *szType, int bitFlags, char *szDesc)
{
    var_link_t  *link, *next;
    cVar        *cvar;

    link = (var_link_t *)mem::alloc( sizeof(var_link_t) );
    cvar = new cVar;

    cvar->Create( szName, szValue, szType, bitFlags, szDesc );

    // alphabetize
    for ( next = m_Head.pNext ; (next != &m_Head) && (stricmp( next->pVariable->getName(), szName ) < 0) ; next = next->pNext );

    link->pNext = next;
    link->pPrev = next->pPrev;

    link->pNext->pPrev = link;
    link->pPrev->pNext = link;

    link->pVariable = cvar;

    return cvar;
}

/*=========================================================
=========================================================*/

void cVariable::Delete (var_link_t *pLink)
{
    pLink->pNext->pPrev = pLink->pPrev;
    pLink->pPrev->pNext = pLink->pNext;

    pLink->pVariable->Destroy( );

    mem::free( pLink->pVariable );
    mem::free( pLink );
}

/*=========================================================
=========================================================*/

void cVar::Create (char *szName, char *szValue, char *szType, int bitFlags, char *szDesc)
{
    int         i;

    for ( i = 0 ; i < cvar_num_types ; i++ )
    {
        if ( stricmp( szType, cvar_type_strings[i] ) == 0 )
        {
            m_Type = (cVarType )i;
            break;
        }
    }
    if ( i == cvar_num_types )
    {
        pMain->Message( "cvar::Create | %s has invalid type %s\n", szName, szType );
        m_Type = cvar_string;
    }

    m_bitFlags = 0;
    setString( szValue );

    strcpy( m_szName, szName );
    m_bitFlags = bitFlags;

    m_szDesc = (char *)mem::alloc( strlen( szDesc )+1 );
    strcpy( m_szDesc, szDesc );
}

void cVar::Destroy ()
{
    mem::free( m_szDesc );
    m_szDesc = NULL;
}

/*=========================================================
=========================================================*/

void cVar::setString (char *szString)
{
    if ( m_bitFlags & CVAR_NOSET )
    {
        printf( "cvar::setString | %s is locked\n", m_szName );
        return;
    }

    switch ( m_Type )
    {
    case cvar_undefined:
    case cvar_string:
        strcpy( m_szValue, szString );
        break;

    case cvar_float:
        setFloat( atof( szString ) );
        break;

    case cvar_int:
        setInt( atoi( szString ) );
        break;

    case cvar_bool:
        if ( strcmp( szString, "true" ) == 0  || atoi( szString ) == 1 )
            setBool( true );
        else
            setBool( false );
        break;

    default:
        printf( "cvar::setString | %s has invalid type %i\n", m_szName, (int)m_Type );
        break;
    }
}

void cVar::setFloat (float f)
{
    if ( m_Type != cvar_float )
        printf( "cvar::setFloat | %s is not type float\n", m_szName );
    else if ( m_bitFlags & CVAR_NOSET )
        printf( "cvar::setFloat | %s is locked\n", m_szName );
    else
    {
        m_Value.f = f;
        fmt( m_szValue, "%f", m_Value.f );
    }
}

void cVar::setInt (int i)
{
    if ( m_Type != cvar_int )
        printf( "cvar::setInt | %s is not type int\n", m_szName );
    else if ( m_bitFlags & CVAR_NOSET )
        printf( "cvar::setInt | %s is locked\n", m_szName );
    else
    {
        m_Value.i = i;
        fmt( m_szValue, "%i", m_Value.i );
    }
}

void cVar::setBool (bool b)
{
    if ( m_Type != cvar_bool )
        printf( "cvar::setBool | %s is not type bool\n", m_szName );
    else if ( m_bitFlags & CVAR_NOSET )
        printf( "cvar::setBool | %s is locked\n", m_szName );
    else if ( b )
    {
        m_Value.b = true;
        strcpy( m_szValue, "true" );
    }
    else
    {
        m_Value.b = false;
        strcpy( m_szValue, "false" );
    }
}

/*=========================================================
=========================================================*/

char *cVar::getString ()
{
    return m_szValue;
}

float cVar::getFloat ()
{
    if ( m_Type == cvar_float )
        return m_Value.f;

    printf( "cvar::getFloat | %s is not type float\n", m_szName );
    return 0;
}

int cVar::getInt ()
{
    if ( m_Type == cvar_int )
        return m_Value.i;

    printf( "cvar::getInt | %s is not type int\n", m_szName );
    return 0;
}

bool cVar::getBool ()
{
    if ( m_Type == cvar_bool )
        return m_Value.b;

    printf( "cvar::getBool | %s is not type bool\n", m_szName );
    return false;
}
