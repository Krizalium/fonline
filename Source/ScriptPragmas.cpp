#include "Common.h"
#include "ScriptPragmas.h"
#include "angelscript.h"
#include "ConstantsManager.h"
#include "Script.h"

// #pragma ignore "other_pragma"
// #pragma other_pragma "arguments" <- not processed
class IgnorePragma
{
private:
    vector< string > ignoredPragmas;

public:
    bool IsIgnored( const string& text )
    {
        return std::find( ignoredPragmas.begin(), ignoredPragmas.end(), text ) != ignoredPragmas.end();
    }

    bool Call( const string& text )
    {
        if( text.length() > 0 )
            ignoredPragmas.push_back( text );
        return true;
    }
};

// #pragma globalvar "int __MyGlobalVar = 100"
class GlobalVarPragma
{
private:
    list< int >           intArray;
    list< int64 >         int64Array;
    list< ScriptString* > stringArray;
    list< float >         floatArray;
    list< double >        doubleArray;
    list< char >          boolArray;

public:
    bool Call( const string& text )
    {
        asIScriptEngine* engine = Script::GetEngine();
        string           type, decl, value;
        char             ch = 0;
        istrstream       str( text.c_str() );
        str >> type >> decl >> ch >> value;

        if( decl == "" )
        {
            WriteLog( "Global var name not found, pragma<%s>.\n", text.c_str() );
            return false;
        }

        int    int_value = ( ch == '=' ? atoi( value.c_str() ) : 0 );
        double float_value = ( ch == '=' ? atof( value.c_str() ) : 0.0 );
        string name = type + " " + decl;

        // Register
        if( type == "int8" || type == "int16" || type == "int32" || type == "int" || type == "uint8" || type == "uint16" || type == "uint32" || type == "uint" )
        {
            auto it = intArray.insert( intArray.begin(), int_value );
            if( engine->RegisterGlobalProperty( name.c_str(), &( *it ) ) < 0 ) WriteLog( "Unable to register integer global var, pragma<%s>.\n", text.c_str() );
        }
        else if( type == "int64" || type == "uint64" )
        {
            auto it = int64Array.insert( int64Array.begin(), int_value );
            if( engine->RegisterGlobalProperty( name.c_str(), &( *it ) ) < 0 ) WriteLog( "Unable to register integer64 global var, pragma<%s>.\n", text.c_str() );
        }
        else if( type == "string" )
        {
            if( value != "" ) value = text.substr( text.find( value ), string::npos );
            auto it = stringArray.insert( stringArray.begin(), ScriptString::Create( value ) );
            if( engine->RegisterGlobalProperty( name.c_str(), ( *it ) ) < 0 ) WriteLog( "Unable to register string global var, pragma<%s>.\n", text.c_str() );
        }
        else if( type == "float" )
        {
            auto it = floatArray.insert( floatArray.begin(), (float) float_value );
            if( engine->RegisterGlobalProperty( name.c_str(), &( *it ) ) < 0 ) WriteLog( "Unable to register float global var, pragma<%s>.\n", text.c_str() );
        }
        else if( type == "double" )
        {
            auto it = doubleArray.insert( doubleArray.begin(), float_value );
            if( engine->RegisterGlobalProperty( name.c_str(), &( *it ) ) < 0 ) WriteLog( "Unable to register double global var, pragma<%s>.\n", text.c_str() );
        }
        else if( type == "bool" )
        {
            value = ( ch == '=' ? value : "false" );
            if( value != "true" && value != "false" )
            {
                WriteLog( "Invalid start value of boolean type, pragma<%s>.\n", text.c_str() );
                return false;
            }
            auto it = boolArray.insert( boolArray.begin(), value == "true" ? true : false );
            if( engine->RegisterGlobalProperty( name.c_str(), &( *it ) ) < 0 ) WriteLog( "Unable to register boolean global var, pragma<%s>.\n", text.c_str() );
        }
        else
        {
            WriteLog( "Global var not registered, unknown type, pragma<%s>.\n", text.c_str() );
        }
        return true;
    }
};

// #pragma bindfunc "int MyObject::MyMethod(int, uint) -> my.dll MyDllFunc"
class BindFuncPragma
{
public:
    bool Call( const string& text )
    {
        asIScriptEngine* engine = Script::GetEngine();
        string           func_name, dll_name, func_dll_name;
        istrstream       str( text.c_str() );
        while( true )
        {
            string s;
            str >> s;
            if( str.fail() || s == "->" )
                break;
            if( func_name != "" )
                func_name += " ";
            func_name += s;
        }
        str >> dll_name;
        str >> func_dll_name;

        if( str.fail() )
        {
            WriteLog( "Error in 'bindfunc' pragma<%s>, parse fail.\n", text.c_str() );
            return false;
        }

        void* dll = Script::LoadDynamicLibrary( dll_name.c_str() );
        if( !dll )
        {
            WriteLog( "Error in 'bindfunc' pragma<%s>, dll not found, error<%s>.\n", text.c_str(), DLL_Error() );
            return false;
        }

        // Find function
        size_t* func = DLL_GetAddress( dll, func_dll_name.c_str() );
        if( !func )
        {
            WriteLog( "Error in 'bindfunc' pragma<%s>, function not found, error<%s>.\n", text.c_str(), DLL_Error() );
            return false;
        }

        int result = 0;
        if( func_name.find( "::" ) == string::npos )
        {
            // Register global function
            result = engine->RegisterGlobalFunction( func_name.c_str(), asFUNCTION( func ), asCALL_CDECL );
        }
        else
        {
            // Register class method
            string::size_type i = func_name.find( " " );
            string::size_type j = func_name.find( "::" );
            if( i == string::npos || i + 1 >= j )
            {
                WriteLog( "Error in 'bindfunc' pragma<%s>, parse class name fail.\n", text.c_str() );
                return false;
            }
            i++;
            string class_name;
            class_name.assign( func_name, i, j - i );
            func_name.erase( i, j - i + 2 );
            result = engine->RegisterObjectMethod( class_name.c_str(), func_name.c_str(), asFUNCTION( func ), asCALL_CDECL_OBJFIRST );
        }
        if( result < 0 )
        {
            WriteLog( "Error in 'bindfunc' pragma<%s>, script registration fail, error<%d>.\n", text.c_str(), result );
            return false;
        }
        return true;
    }
};

// #pragma entity EntityName Movable = true
#include "Entity.h"
#ifdef FONLINE_SERVER
# include "EntityManager.h"
#endif

class EntityCreator
{
public:
    EntityCreator( bool is_server, const char* class_name )
    {
        ClassName = class_name;
        Registrator = new PropertyRegistrator( is_server, class_name );
        static uint sub_type_id = 0;
        SubType = ++sub_type_id;
    }

    string               ClassName;
    PropertyRegistrator* Registrator;
    uint                 SubType;

    #ifdef FONLINE_SERVER
    CustomEntity* CreateEntity()
    {
        CustomEntity* entity = new CustomEntity( Entity::GenerateId, SubType, Registrator );
        EntityMngr.RegisterEntity( entity );
        return entity;
    }

    void RestoreEntity( uint id, Properties& props )
    {
        CustomEntity* entity = new CustomEntity( id, SubType, Registrator );
        entity->Props = props;
        EntityMngr.RegisterEntity( entity );
    }

    void DeleteEntity( CustomEntity* entity )
    {
        RUNTIME_ASSERT( entity->SubType == SubType );
        entity->IsDestroyed = true;
        EntityMngr.UnregisterEntity( entity );
    }

    void DeleteEntityById( uint id )
    {
        CustomEntity* entity = (CustomEntity*) EntityMngr.GetEntity( id, EntityType::Custom );
        if( !entity || entity->SubType != SubType )
            return;
        entity->IsDestroyed = true;
        EntityMngr.UnregisterEntity( entity );
    }

    CustomEntity* GetEntity( uint id )
    {
        CustomEntity* entity = (CustomEntity*) EntityMngr.GetEntity( id, EntityType::Custom );
        if( entity && entity->SubType == SubType )
            return entity;
        return NULL;
    }

    #else
    CustomEntity* CreateEntity()                              { return NULL; }
    void          RestoreEntity( uint id, Properties& props ) {}
    void          DeleteEntity( CustomEntity* entity )        {}
    void          DeleteEntityById( uint id )                 {}
    CustomEntity* GetEntity( uint id )                        { return NULL; }
    #endif
};

class EntityPragma
{
    friend class PropertyPragma;

private:
    map< string, EntityCreator* > entityCreators;
    bool                          isServer;

public:
    EntityPragma( int pragma_type )
    {
        isServer = ( pragma_type == PRAGMA_SERVER || pragma_type == PRAGMA_MAPPER );
    }

    ~EntityPragma()
    {
        for( auto it = entityCreators.begin(); it != entityCreators.end(); ++it )
            SAFEDEL( it->second );
        entityCreators.clear();
    }

    bool Call( const string& text )
    {
        // Read all
        istrstream str( text.c_str() );
        string     class_name;
        str >> class_name;

        // Check already registered classes
        asIScriptEngine* engine = Script::GetEngine();
        if( engine->GetObjectTypeByName( class_name.c_str() ) )
        {
            WriteLog( "Error in 'entity' pragma '%s', class already registered.\n", text.c_str() );
            return false;
        }

        // Create object
        if( engine->RegisterObjectType( class_name.c_str(), 0, asOBJ_REF ) < 0 ||
            engine->RegisterObjectBehaviour( class_name.c_str(), asBEHAVE_ADDREF, "void f()", asMETHOD( Entity, AddRef ), asCALL_THISCALL ) < 0 ||
            engine->RegisterObjectBehaviour( class_name.c_str(), asBEHAVE_RELEASE, "void f()", asMETHOD( Entity, Release ), asCALL_THISCALL ) < 0 ||
            engine->RegisterObjectProperty( class_name.c_str(), "const uint Id", OFFSETOF( Entity, Id ) ) < 0 ||
            engine->RegisterObjectProperty( class_name.c_str(), "const bool IsDestroyed", OFFSETOF( Entity, IsDestroyed ) ) < 0 ||
            engine->RegisterObjectProperty( class_name.c_str(), "const bool IsDestroying", OFFSETOF( Entity, IsDestroying ) ) < 0 )
        {
            WriteLog( "Error in 'entity' pragma '%s', fail to register object type.\n", text.c_str() );
            return false;
        }

        // Create creator
        EntityCreator* entity_creator = new EntityCreator( isServer, class_name.c_str() );
        if( isServer )
        {
            char buf[ MAX_FOTEXT ];
            if( engine->RegisterGlobalFunction( Str::Format( buf, "%s@+ Create%s()", class_name.c_str(), class_name.c_str() ), asMETHOD( EntityCreator, CreateEntity ), asCALL_THISCALL_ASGLOBAL, entity_creator ) < 0 ||
                engine->RegisterGlobalFunction( Str::Format( buf, "void Delete%s(%s& entity)", class_name.c_str(), class_name.c_str() ), asMETHOD( EntityCreator, DeleteEntity ), asCALL_THISCALL_ASGLOBAL, entity_creator ) < 0 ||
                engine->RegisterGlobalFunction( Str::Format( buf, "void Delete%s(uint id)", class_name.c_str() ), asMETHOD( EntityCreator, DeleteEntityById ), asCALL_THISCALL_ASGLOBAL, entity_creator ) < 0 ||
                engine->RegisterGlobalFunction( Str::Format( buf, "%s@+ Get%s(uint id)", class_name.c_str(), class_name.c_str() ), asMETHOD( EntityCreator, GetEntity ), asCALL_THISCALL_ASGLOBAL, entity_creator ) < 0 )
            {
                WriteLog( "Error in 'entity' pragma '%s', fail to register management functions.\n", text.c_str() );
                return false;
            }
        }

        // Init registrator
        if( !entity_creator->Registrator->Init() )
        {
            WriteLog( "Error in 'entity' pragma '%s', fail to initialize property registrator.\n", text.c_str() );
            return false;
        }

        // Add creator
        RUNTIME_ASSERT( !entityCreators.count( class_name ) );
        entityCreators.insert( PAIR( class_name, entity_creator ) );
        return true;
    }

    bool Finish()
    {
        for( auto it = entityCreators.begin(); it != entityCreators.end(); ++it )
            it->second->Registrator->FinishRegistration();
        return true;
    }

    PropertyRegistrator* FindEntityRegistrator( const char* class_name )
    {
        auto it = entityCreators.find( class_name );
        return it != entityCreators.end() ? it->second->Registrator : NULL;
    }

    void RestoreEntity( const char* class_name, uint id, Properties& props )
    {
        EntityCreator* entity_creator = entityCreators[ class_name ];
        entity_creator->RestoreEntity( id, props );
    }
};

// #pragma property "MyObject Virtual int PropName Default = 100 Min = 10 Max = 50 Random = true"
class PropertyPragma
{
private:
    PropertyRegistrator** propertyRegistrators;
    EntityPragma*         entitiesRegistrators;

public:
    PropertyPragma( bool is_server, EntityPragma* entities_registrators )
    {
        propertyRegistrators = new PropertyRegistrator*[ 6 ];
        propertyRegistrators[ 0 ] = new PropertyRegistrator( is_server, "GlobalVars" );
        propertyRegistrators[ 1 ] = new PropertyRegistrator( is_server, is_server ? "Critter" : "CritterCl" );
        propertyRegistrators[ 2 ] = new PropertyRegistrator( is_server, is_server ? "Item" : "ItemCl" );
        propertyRegistrators[ 3 ] = new PropertyRegistrator( is_server, "ProtoItem" );
        propertyRegistrators[ 4 ] = new PropertyRegistrator( is_server, "Map" );
        propertyRegistrators[ 5 ] = new PropertyRegistrator( is_server, "Location" );

        entitiesRegistrators = entities_registrators;
    }

    ~PropertyPragma()
    {
        for( int i = 0; i < 6; i++ )
            SAFEDEL( propertyRegistrators[ i ] );
        SAFEDEL( propertyRegistrators );
    }

    PropertyRegistrator** GetPropertyRegistrators()
    {
        return propertyRegistrators;
    }

    bool Call( const string& text )
    {
        // Read all
        istrstream str( text.c_str() );
        string     class_name, access_str, property_type_name, property_name;
        str >> class_name >> access_str;

        // Defaults
        bool is_defaults = false;
        if( Str::CompareCase( access_str.c_str(), "Defaults" ) )
            is_defaults = true;

        // Property type and name
        if( !is_defaults )
        {
            str >> property_type_name >> property_name;
            if( property_name == "<" && !str.fail() )
            {
                while( property_name != ">" && !str.fail() )
                {
                    property_type_name += property_name;
                    str >> property_name;
                }
                property_type_name += property_name;
                str >> property_name;
            }
            if( property_name == "[" && !str.fail() )
            {
                while( property_name != "]" && !str.fail() )
                {
                    property_type_name += property_name;
                    str >> property_name;
                }
                property_type_name += property_name;
                str >> property_name;
            }
        }

        if( str.fail() )
        {
            WriteLog( "Error in 'property' pragma<%s>.\n", text.c_str() );
            return false;
        }

        char options_buf[ MAX_FOTEXT ] = { 0 };
        str.getline( options_buf, MAX_FOTEXT );

        // Parse access type
        Property::AccessType access = (Property::AccessType) 0;
        if( !is_defaults )
        {
            if( Str::CompareCase( access_str.c_str(), "PrivateCommon" ) )
                access = Property::PrivateCommon;
            else if( Str::CompareCase( access_str.c_str(), "PrivateClient" ) )
                access = Property::PrivateClient;
            else if( Str::CompareCase( access_str.c_str(), "PrivateServer" ) )
                access = Property::PrivateServer;
            else if( Str::CompareCase( access_str.c_str(), "Public" ) )
                access = Property::Public;
            else if( Str::CompareCase( access_str.c_str(), "PublicModifiable" ) )
                access = Property::PublicModifiable;
            else if( Str::CompareCase( access_str.c_str(), "Protected" ) )
                access = Property::Protected;
            else if( Str::CompareCase( access_str.c_str(), "ProtectedModifiable" ) )
                access = Property::ProtectedModifiable;
            else if( Str::CompareCase( access_str.c_str(), "VirtualPrivateCommon" ) )
                access = Property::VirtualPrivateCommon;
            else if( Str::CompareCase( access_str.c_str(), "VirtualPrivateClient" ) )
                access = Property::VirtualPrivateClient;
            else if( Str::CompareCase( access_str.c_str(), "VirtualPrivateServer" ) )
                access = Property::VirtualPrivateServer;
            else if( Str::CompareCase( access_str.c_str(), "VirtualPublic" ) )
                access = Property::VirtualPublic;
            else if( Str::CompareCase( access_str.c_str(), "VirtualProtected" ) )
                access = Property::VirtualProtected;
            if( access == (Property::AccessType) 0 )
            {
                WriteLog( "Error in 'property' pragma<%s>, invalid access type<%s>.\n", text.c_str(), access_str.c_str() );
                return false;
            }
        }

        // Parse options
        string group;
        bool   generate_random_value = false;
        bool   set_default_value = false;
        int64  default_value = 0;
        bool   check_min_value = false;
        int64  min_value = 0;
        bool   check_max_value = false;
        int64  max_value = 0;
        string get_callback;
        StrVec set_callbacks;
        StrVec opt_entries;
        Str::ParseLine( options_buf, ',', opt_entries, Str::ParseLineDummy );
        for( size_t i = 0, j = opt_entries.size(); i < j; i++ )
        {
            StrVec opt_entry;
            Str::ParseLine( opt_entries[ i ].c_str(), '=', opt_entry, Str::ParseLineDummy );
            if( opt_entry.size() != 2 )
            {
                WriteLog( "Error in 'property' pragma<%s>, invalid options entry.\n", text.c_str() );
                return false;
            }

            const char*   opt_name = opt_entry[ 0 ].c_str();
            const string& opt_svalue = opt_entry[ 1 ];
            int64         opt_ivalue = ConvertParamValue( opt_svalue.c_str() );

            if( Str::CompareCase( opt_name, "Group" ) )
            {
                group = opt_svalue;
            }
            else if( Str::CompareCase( opt_name, "Default" ) )
            {
                set_default_value = true;
                default_value = opt_ivalue;
            }
            else if( Str::CompareCase( opt_name, "Min" ) )
            {
                check_min_value = true;
                min_value = opt_ivalue;
            }
            else if( Str::CompareCase( opt_name, "Max" ) )
            {
                check_max_value = true;
                max_value = opt_ivalue;
            }
            else if( Str::CompareCase( opt_name, "Random" ) )
            {
                generate_random_value = ( opt_ivalue != 0 );
            }
            else if( Str::CompareCase( opt_name, "GetCallabck" ) )
            {
                get_callback = opt_svalue;
            }
            else if( Str::CompareCase( opt_name, "SetCallabck" ) )
            {
                set_callbacks.push_back( opt_svalue );
            }
        }

        // Choose registrator
        PropertyRegistrator* registrator = NULL;
        if( class_name == "Global" )
            registrator = propertyRegistrators[ 0 ];
        else if( class_name == "Critter" )
            registrator = propertyRegistrators[ 1 ];
        else if( class_name == "Item" )
            registrator = propertyRegistrators[ 2 ];
        else if( class_name == "ProtoItem" )
            registrator = propertyRegistrators[ 3 ];
        else if( class_name == "Map" )
            registrator = propertyRegistrators[ 4 ];
        else if( class_name == "Location" )
            registrator = propertyRegistrators[ 5 ];
        else if( entitiesRegistrators->entityCreators.count( class_name ) )
            registrator = entitiesRegistrators->entityCreators[ class_name ]->Registrator;
        else
            WriteLog( "Invalid class in 'property' pragma<%s>.\n", text.c_str() );
        if( !registrator )
            return false;

        // Register
        if( !is_defaults )
        {
            if( !registrator->Register( property_type_name.c_str(), property_name.c_str(), access,
                                        group.length() > 0 ? group.c_str() : NULL, generate_random_value ? &generate_random_value : NULL,
                                        set_default_value ? &default_value : NULL, check_min_value ? &min_value : NULL, check_max_value ? &max_value : NULL ) )
            {
                WriteLog( "Unable to register 'property' pragma<%s>.\n", text.c_str() );
                return false;
            }
        }

        // Work with defaults
        if( is_defaults )
        {
            registrator->SetDefaults( group.length() > 0 ? group.c_str() : NULL, generate_random_value ? &generate_random_value : NULL,
                                      set_default_value ? &default_value : NULL, check_min_value ? &min_value : NULL, check_max_value ? &max_value : NULL );
        }

        return true;
    }

    bool Finish()
    {
        return true;
    }
};

// #pragma method MyObject CallType void Foo(int a, int b) FuncToBind
class MethodPragma
{
private:
    MethodRegistrator** methodRegistrator;
    EntityPragma*       entitiesRegistrators;

public:
    MethodPragma( bool is_server, EntityPragma* entities_registrators )
    {
        methodRegistrator = new MethodRegistrator*[ 6 ];
        methodRegistrator[ 0 ] = new MethodRegistrator( is_server, "GlobalVars" );
        methodRegistrator[ 1 ] = new MethodRegistrator( is_server, is_server ? "Critter" : "CritterCl" );
        methodRegistrator[ 2 ] = new MethodRegistrator( is_server, is_server ? "Item" : "ItemCl" );
        methodRegistrator[ 3 ] = new MethodRegistrator( is_server, "ProtoItem" );
        methodRegistrator[ 4 ] = new MethodRegistrator( is_server, "Map" );
        methodRegistrator[ 5 ] = new MethodRegistrator( is_server, "Location" );

        entitiesRegistrators = entities_registrators;
    }

    bool Call( const string& text )
    {
        // Read all
        istrstream str( text.c_str() );
        string     class_name, call_type_str;
        str >> class_name >> call_type_str;
        char       buf[ MAX_FOTEXT ];
        str.getline( buf, MAX_FOTEXT );
        if( str.fail() )
        {
            WriteLog( "Error in 'method' pragma '%s'.\n", text.c_str() );
            return false;
        }

        // Decl
        char* separator = Str::Substring( buf, "=" );
        if( !separator )
        {
            WriteLog( "Error in 'method' pragma declaration '%s'.\n", text.c_str() );
            return false;
        }
        *separator = 0;

        char decl[ MAX_FOTEXT ];
        char script_func[ MAX_FOTEXT ];
        Str::Copy( decl, buf );
        Str::Copy( script_func, separator + 1 );
        Str::Trim( decl );
        Str::Trim( script_func );

        // Parse access type
        Method::CallType call_type = (Method::CallType) 0;
        if( Str::CompareCase( call_type_str.c_str(), "LocalServer" ) )
            call_type = Method::LocalServer;
        else if( Str::CompareCase( call_type_str.c_str(), "LocalClient" ) )
            call_type = Method::LocalClient;
        else if( Str::CompareCase( call_type_str.c_str(), "LocalCommon" ) )
            call_type = Method::LocalCommon;
        else if( Str::CompareCase( call_type_str.c_str(), "RemoteServer" ) )
            call_type = Method::RemoteServer;
        else if( Str::CompareCase( call_type_str.c_str(), "RemoteClient" ) )
            call_type = Method::RemoteClient;
        if( call_type == (Method::CallType) 0 )
        {
            WriteLog( "Error in 'method' pragma '%s', invalid call type '%s'.\n", text.c_str(), call_type_str.c_str() );
            return false;
        }

        // Choose registrator
        MethodRegistrator* registrator = NULL;
        if( class_name == "Global" )
            registrator = methodRegistrator[ 0 ];
        else if( class_name == "Critter" )
            registrator = methodRegistrator[ 1 ];
        else if( class_name == "Item" )
            registrator = methodRegistrator[ 2 ];
        else if( class_name == "ProtoItem" )
            registrator = methodRegistrator[ 3 ];
        else if( class_name == "Map" )
            registrator = methodRegistrator[ 4 ];
        else if( class_name == "Location" )
            registrator = methodRegistrator[ 5 ];
        // else if (entitiesRegistrators->entityCreators.count(class_name))
        //	registrator = entitiesRegistrators->entityCreators[class_name]->Registrator;
        else
            WriteLog( "Invalid class in 'property' pragma<%s>.\n", text.c_str() );
        if( !registrator )
            return false;

        // Register
        if( !registrator->Register( decl, script_func, call_type ) )
        {
            WriteLog( "Unable to register 'method' pragma '%s'.\n", text.c_str() );
            return false;
        }

        return true;
    }

    bool Finish()
    {
        return true; // methodRegistrator->FinishRegistration();
    }
};

// #pragma content Group fileName
#ifdef FONLINE_SERVER
# include "Dialogs.h"
# include "ItemManager.h"
# include "CritterManager.h"
# include "MapManager.h"
#endif

class ContentPragma
{
private:
    list< hash > dataStorage;
    StrUIntMap   filesToCheck[ 5 ];

public:
    bool Call( const string& text )
    {
        // Read all
        istrstream str( text.c_str() );
        string     group, file_name;
        str >> group >> file_name;
        if( str.fail() )
        {
            WriteLog( "Unable to parse 'content' pragma<%s>.\n", text.c_str() );
            return false;
        }

        // Make hash
        hash h = Str::GetHash( file_name.c_str() );

        // Verify file
        asIScriptEngine* engine = Script::GetEngine();
        int              group_index;
        const char*      ns;
        if( Str::CompareCase( group.c_str(), "Dialog" ) )
        {
            group_index = 0;
            ns = "Content::Dialog";
        }
        else if( Str::CompareCase( group.c_str(), "Item" ) )
        {
            group_index = 1;
            ns = "Content::Item";
        }
        else if( Str::CompareCase( group.c_str(), "Critter" ) )
        {
            group_index = 2;
            ns = "Content::Critter";
        }
        else if( Str::CompareCase( group.c_str(), "Location" ) )
        {
            group_index = 3;
            ns = "Content::Location";
        }
        else if( Str::CompareCase( group.c_str(), "Map" ) )
        {
            group_index = 4;
            ns = "Content::Map";
        }
        else
        {
            WriteLog( "Invalid group name in 'content' pragma<%s>.\n", text.c_str() );
            return false;
        }

        if( filesToCheck[ group_index ].count( file_name ) )
            return true;
        filesToCheck[ group_index ].insert( PAIR( file_name, h ) );

        // Register file
        dataStorage.push_back( h );
        engine->SetDefaultNamespace( "Content" );
        engine->SetDefaultNamespace( ns );
        char prop_name[ MAX_FOTEXT ];
        Str::Format( prop_name, "const hash %s", file_name.c_str() );
        int  result = engine->RegisterGlobalProperty( prop_name, &dataStorage.back() );
        engine->SetDefaultNamespace( "" );
        if( result < 0 )
        {
            WriteLog( "Error in 'content' pragma<%s>, error<%d>.\n", text.c_str(), result );
            return false;
        }

        return true;
    }

    bool Finish()
    {
        int errors = 0;
        #ifdef FONLINE_SERVER
        for( auto it = filesToCheck[ 0 ].begin(); it != filesToCheck[ 0 ].end(); ++it )
        {
            if( !DlgMngr.GetDialog( it->second ) )
            {
                WriteLog( "Dialog file '%s' not found.\n", it->first.c_str() );
                errors++;
            }
        }
        for( auto it = filesToCheck[ 1 ].begin(); it != filesToCheck[ 1 ].end(); ++it )
        {
            if( !ItemMngr.GetProtoItem( it->second ) )
            {
                WriteLog( "Item file '%s' not found.\n", it->first.c_str() );
                errors++;
            }
        }
        for( auto it = filesToCheck[ 2 ].begin(); it != filesToCheck[ 2 ].end(); ++it )
        {
            if( !CrMngr.GetProto( it->second ) )
            {
                WriteLog( "Critter file '%s' not found.\n", it->first.c_str() );
                errors++;
            }
        }
        for( auto it = filesToCheck[ 3 ].begin(); it != filesToCheck[ 3 ].end(); ++it )
        {
            if( !MapMngr.GetProtoLocation( it->second ) )
            {
                WriteLog( "Location file '%s' not found.\n", it->first.c_str() );
                errors++;
            }
        }
        for( auto it = filesToCheck[ 4 ].begin(); it != filesToCheck[ 4 ].end(); ++it )
        {
            if( !MapMngr.GetProtoMap( it->second ) )
            {
                WriteLog( "Map file '%s' not found.\n", it->first.c_str() );
                errors++;
            }
        }
        #endif
        return errors == 0;
    }
};

ScriptPragmaCallback::ScriptPragmaCallback( int pragma_type )
{
    isError = false;
    pragmaType = pragma_type;
    if( pragmaType != PRAGMA_SERVER && pragmaType != PRAGMA_CLIENT && pragmaType != PRAGMA_MAPPER )
        pragmaType = PRAGMA_UNKNOWN;

    ignorePragma = NULL;
    globalVarPragma = NULL;
    bindFuncPragma = NULL;
    entityPragma = NULL;
    propertyPragma = NULL;
    methodPragma = NULL;
    contentPragma = NULL;

    if( pragmaType != PRAGMA_UNKNOWN )
    {
        ignorePragma = new IgnorePragma();
        globalVarPragma = new GlobalVarPragma();
        bindFuncPragma = new BindFuncPragma();
        entityPragma = new EntityPragma( pragmaType );
        propertyPragma = new PropertyPragma( pragmaType == PRAGMA_SERVER || pragmaType == PRAGMA_MAPPER, entityPragma );
        methodPragma = new MethodPragma( pragmaType == PRAGMA_SERVER || pragmaType == PRAGMA_MAPPER, entityPragma );
        contentPragma = new ContentPragma();
    }
}

ScriptPragmaCallback::~ScriptPragmaCallback()
{
    SAFEDEL( ignorePragma );
    SAFEDEL( globalVarPragma );
    SAFEDEL( bindFuncPragma );
    SAFEDEL( entityPragma );
    SAFEDEL( propertyPragma );
    SAFEDEL( methodPragma );
    SAFEDEL( contentPragma );
}

void ScriptPragmaCallback::CallPragma( const Preprocessor::PragmaInstance& pragma )
{
    string pragma_instance = pragma.CurrentFile + Str::ItoA( pragma.CurrentFileLine );
    if( alreadyProcessed.count( pragma_instance ) )
        return;
    alreadyProcessed.insert( pragma_instance );

    if( ignorePragma && ignorePragma->IsIgnored( pragma.Name ) )
        return;

    bool ok = false;
    if( Str::CompareCase( pragma.Name.c_str(), "ignore" ) && ignorePragma )
        ok = ignorePragma->Call( pragma.Text );
    else if( Str::CompareCase( pragma.Name.c_str(), "globalvar" ) && globalVarPragma )
        ok = globalVarPragma->Call( pragma.Text );
    else if( Str::CompareCase( pragma.Name.c_str(), "bindfunc" ) && bindFuncPragma )
        ok = bindFuncPragma->Call( pragma.Text );
    else if( Str::CompareCase( pragma.Name.c_str(), "property" ) && propertyPragma )
        ok = propertyPragma->Call( pragma.Text );
    else if( Str::CompareCase( pragma.Name.c_str(), "method" ) && methodPragma )
        ok = methodPragma->Call( pragma.Text );
    else if( Str::CompareCase( pragma.Name.c_str(), "entity" ) && entityPragma )
        ok = entityPragma->Call( pragma.Text );
    else if( Str::CompareCase( pragma.Name.c_str(), "content" ) && contentPragma )
        ok = contentPragma->Call( pragma.Text );
    else
        WriteLog( "Unknown pragma instance, name<%s> text<%s>.\n", pragma.Name.c_str(), pragma.Text.c_str() ), ok = false;

    if( ok )
        processedPragmas.push_back( pragma );
    else
        isError = true;
}

const Pragmas& ScriptPragmaCallback::GetProcessedPragmas()
{
    return processedPragmas;
}

void ScriptPragmaCallback::Finish()
{
    if( !entityPragma->Finish() )
        isError = true;
    if( !propertyPragma->Finish() )
        isError = true;
    if( !methodPragma->Finish() )
        isError = true;
    if( !contentPragma->Finish() )
        isError = true;
}

bool ScriptPragmaCallback::IsError()
{
    return isError;
}

PropertyRegistrator** ScriptPragmaCallback::GetPropertyRegistrators()
{
    return propertyPragma->GetPropertyRegistrators();
}

PropertyRegistrator* ScriptPragmaCallback::FindEntityRegistrator( const char* class_name )
{
    return entityPragma->FindEntityRegistrator( class_name );
}

void ScriptPragmaCallback::RestoreEntity( const char* class_name, uint id, Properties& props )
{
    entityPragma->RestoreEntity( class_name, id, props );
}
