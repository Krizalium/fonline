#include "Common.h"
#include "Client.h"
#include "Exception.h"
#include "Keyboard.h"
#include <locale.h>
#ifndef FO_WINDOWS
# include <signal.h>
#endif

extern "C" int main( int argc, char** argv ) // Handled by SDL
{
    InitialSetup( argc, argv );

    // Disable SIGPIPE signal
    #ifndef FO_WINDOWS
    signal( SIGPIPE, SIG_IGN );
    #endif

    // Exception
    CatchExceptions( "FOnline", FONLINE_VERSION );

    // Timer
    Timer::Init();

    // Logging
    LogToFile( "FOnline.log" );
    LogToDebugOutput( true );

    // Singleplayer mode initialization
    #ifdef FO_WINDOWS
    char full_path[ MAX_FOPATH ] = { 0 };
    char path[ MAX_FOPATH ] = { 0 };
    char name[ MAX_FOPATH ] = { 0 };
    GetModuleFileName( nullptr, full_path, MAX_FOPATH );
    FileManager::ExtractDir( full_path, path );
    FileManager::ExtractFileName( full_path, name );
    if( Str::Substring( name, "Singleplayer" ) || MainConfig->IsKey( "", "Singleplayer" ) )
    {
        WriteLog( "Singleplayer mode.\n" );
        Singleplayer = true;
        GameOpt.Singleplayer = true;
        Timer::SetGamePause( true );

        // Create interprocess shared data
        HANDLE map_file = SingleplayerData.Init();
        if( !map_file )
        {
            WriteLog( "Can't map shared data to memory.\n" );
            return 0;
        }

        // Fill interprocess initial data
        if( SingleplayerData.Lock() )
        {
            // Initialize other data
            SingleplayerData.NetPort = 0;
            SingleplayerData.Pause = true;

            SingleplayerData.Unlock();
        }
        else
        {
            WriteLog( "Can't lock mapped file.\n" );
            return 0;
        }

        // Config parsing
        const char* server_exe = "FOnlineServer.exe";
        const char* server_dir = MainConfig->GetStr( "", "ServerDir", "" );
        const char* server_cmdline = MainConfig->GetStr( "", "ServerCommandLine", "" );

        // Process attributes
        PROCESS_INFORMATION server;
        memzero( &server, sizeof( server ) );
        STARTUPINFOA        sui;
        memzero( &sui, sizeof( sui ) );
        sui.cb = sizeof( sui );
        HANDLE client_process = OpenProcess( SYNCHRONIZE, TRUE, GetCurrentProcessId() );
        char   command_line[ 2048 ];

        // Start server
        Str::Format( command_line, "\"%s%s\" -singleplayer %p %p %s -logpath %s", server_dir, server_exe, map_file, client_process, server_cmdline, path );
        if( !CreateProcess( nullptr, command_line, nullptr, nullptr, TRUE, NORMAL_PRIORITY_CLASS, nullptr, server_dir, &sui, &server ) )
        {
            WriteLog( "Can't start server process, error {}.\n", GetLastError() );
            return 0;
        }
        CloseHandle( server.hProcess );
        CloseHandle( server.hThread );
    }
    #endif

    // Check for already runned window
    #ifndef DEV_VERSION
    # ifdef FO_WINDOWS
    HANDLE h = CreateEvent( nullptr, FALSE, FALSE, "fonline_instance" );
    if( !h || h == INVALID_HANDLE_VALUE || GetLastError() == ERROR_ALREADY_EXISTS )
    {
        ShowMessage( "FOnline engine instance is already running." );
        return 0;
    }
    # else
    // Todo: Linux
    # endif
    #endif

    // Options
    GetClientOptions();

    // Start message
    WriteLog( "Starting FOnline (version {})...\n", FONLINE_VERSION );

    // Create engine
    FOClient* engine = new FOClient();

    // iOS or Web loop
    #if defined ( FO_IOS ) || defined ( FO_WEB )
    struct App
    {
        static void ShowFrame( void* engine )
        {
            ( (FOClient*) engine )->MainLoop();
        }
    };
    # ifdef FO_IOS
    SDL_iPhoneSetAnimationCallback( MainWindow, 1, App::ShowFrame, engine );
    # else
    emscripten_set_main_loop_arg( App::ShowFrame, engine, 60, 1 );
    # endif
    return 0;

    #else
    // Default loop
    while( !GameOpt.Quit )
    {
        double start_loop = Timer::AccurateTick();

        engine->MainLoop();

        if( !GameOpt.VSync && GameOpt.FixedFPS )
        {
            if( GameOpt.FixedFPS > 0 )
            {
                static double balance = 0.0;
                double        elapsed = Timer::AccurateTick() - start_loop;
                double        need_elapsed = 1000.0 / (double) GameOpt.FixedFPS;
                if( need_elapsed > elapsed )
                {
                    double sleep = need_elapsed - elapsed + balance;
                    balance = fmod ( sleep, 1.0 );
                    Thread_Sleep( (uint) floor( sleep ) );
                }
            }
            else
            {
                Thread_Sleep( -GameOpt.FixedFPS );
            }
        }
    }
    #endif

    // Finish script
    Script::RunMandatorySuspended();
    Script::RaiseInternalEvent( ClientFunctions.Finish, _FUNC_, "Game" );

    // Just kill process
    // System automatically clean up all resources
    WriteLog( "Exit from game.\n" );
    ExitProcess( 0 );

    // Destroy engine
    engine->Finish();
    SAFEDEL( engine );

    // Finish
    #ifdef FO_WINDOWS
    if( Singleplayer )
        SingleplayerData.Finish();
    #endif
    WriteLog( "FOnline finished.\n" );
    LogFinish();

    return 0;
}
