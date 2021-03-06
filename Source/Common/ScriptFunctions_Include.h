//      __________        ___               ______            _
//     / ____/ __ \____  / (_)___  ___     / ____/___  ____ _(_)___  ___
//    / /_  / / / / __ \/ / / __ \/ _ \   / __/ / __ \/ __ `/ / __ \/ _ \
//   / __/ / /_/ / / / / / / / / /  __/  / /___/ / / / /_/ / / / / /  __/
//  /_/    \____/_/ /_/_/_/_/ /_/\___/  /_____/_/ /_/\__, /_/_/ /_/\___/
//                                                  /____/
// FOnline Engine
// https://fonline.ru
// https://github.com/cvet/fonline
//
// MIT License
//
// Copyright (c) 2006 - present, Anton Tsvetinskiy aka cvet <cvet@tut.by>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

// Common functions for server, client, mapper
// Works in scripts compiler

template<class T>
static Entity* EntityDownCast(T* a)
{
    if (!a)
        return nullptr;
    Entity* b = (Entity*)a;
    b->AddRef();
    return b;
}

template<class T>
static T* EntityUpCast(Entity* a)
{
    if (!a)
        return nullptr;
#define CHECK_CAST(cast_class, entity_type) \
    if (std::is_same<T, cast_class>::value && (EntityType)a->Type == entity_type) \
    { \
        T* b = (T*)a; \
        b->AddRef(); \
        return b; \
    }
        // Todo: rework FONLINE_
        /*CHECK_CAST(Location, EntityType::Location);
        CHECK_CAST(Map, EntityType::Map);
        CHECK_CAST(Critter, EntityType::Npc);
        CHECK_CAST(Critter, EntityType::Client);
        CHECK_CAST(Item, EntityType::Item);
        CHECK_CAST(LocationView, EntityType::LocationView);
        CHECK_CAST(MapView, EntityType::MapView);
        CHECK_CAST(CritterView, EntityType::CritterView);
        CHECK_CAST(ItemView, EntityType::ItemView);
        CHECK_CAST(ItemView, EntityType::ItemHexView);*/
#undef CHECK_CAST
    return nullptr;
}

static void Global_Assert(bool condition)
{
    // Todo: fix script system
    // if (!condition)
    //    Script::RaiseException("Assertion failed");
}

static void Global_ThrowException(string message)
{
    // Todo: fix script system
    // Script::RaiseException(message);
}

static int Global_Random(int min, int max)
{
    return GenericUtils::Random(min, max);
}

static void Global_Log(string text)
{
    // Todo: fix script system
    // Script::Log(text);
}

static bool Global_StrToInt(string text, int& result)
{
    if (!_str(text).isNumber())
        return false;
    result = _str(text).toInt();
    return true;
}

static bool Global_StrToFloat(string text, float& result)
{
    if (!_str(text).isNumber())
        return false;
    result = _str(text).toFloat();
    return true;
}

static uint Global_GetDistantion(ushort hx1, ushort hy1, ushort hx2, ushort hy2)
{
    // Todo: need attention!
    // return DistGame(hx1, hy1, hx2, hy2);
    return 0;
}

static uchar Global_GetDirection(ushort from_hx, ushort from_hy, ushort to_hx, ushort to_hy)
{
    // Todo: need attention!
    // return GetFarDir(from_hx, from_hy, to_hx, to_hy);
    return 0;
}

static uchar Global_GetOffsetDir(ushort from_hx, ushort from_hy, ushort to_hx, ushort to_hy, float offset)
{
    // Todo: need attention!
    // return GetFarDir(from_hx, from_hy, to_hx, to_hy, offset);
    return 0;
}

static uint Global_GetTick()
{
    return Timer::FastTick();
}

static uint Global_GetAngelScriptProperty(int property)
{
    return (uint)asGetActiveContext()->GetEngine()->GetEngineProperty((asEEngineProp)property);
}

static bool Global_SetAngelScriptProperty(int property, uint value)
{
    return asGetActiveContext()->GetEngine()->SetEngineProperty((asEEngineProp)property, value) >= 0;
}

static hash Global_GetStrHash(string str)
{
    if (str.empty())
        return 0;
    return _str(str).toHash();
}

static string Global_GetHashStr(hash h)
{
    return _str().parseHash(h);
}

static uint Global_DecodeUTF8(string text, uint& length)
{
    return utf8::Decode(text.c_str(), &length);
}

static string Global_EncodeUTF8(uint ucs)
{
    char buf[4];
    uint len = utf8::Encode(ucs, buf);
    return string(buf, len);
}

static void Global_Yield(uint time)
{
    // Todo: fix script system
    // Script::SuspendCurrentContext(time);
}

static string Global_SHA1(string text)
{
    SHA1_CTX ctx;
    _SHA1_Init(&ctx);
    _SHA1_Update(&ctx, (uchar*)text.c_str(), text.length());
    uchar digest[SHA1_DIGEST_SIZE];
    _SHA1_Final(&ctx, digest);

    static const char* nums = "0123456789abcdef";
    char hex_digest[SHA1_DIGEST_SIZE * 2];
    for (uint i = 0; i < sizeof(hex_digest); i++)
        hex_digest[i] = nums[i % 2 ? digest[i / 2] & 0xF : digest[i / 2] >> 4];
    return string(hex_digest, sizeof(hex_digest));
}

static string Global_SHA2(string text)
{
    const uint digest_size = 32;
    uchar digest[digest_size];
    sha256((uchar*)text.c_str(), (uint)text.length(), digest);

    static const char* nums = "0123456789abcdef";
    char hex_digest[digest_size * 2];
    for (uint i = 0; i < sizeof(hex_digest); i++)
        hex_digest[i] = nums[i % 2 ? digest[i / 2] & 0xF : digest[i / 2] >> 4];
    return string(hex_digest, sizeof(hex_digest));
}

static void PrintLog(string& log, bool last_call, std::function<void(const string&)> log_callback)
{
    // Normalize new lines to \n
    while (true)
    {
        size_t pos = log.find("\r\n");
        if (pos != string::npos)
            log.replace(pos, 2, "\n");
        else
            break;
    }
    log.erase(std::remove(log.begin(), log.end(), '\r'), log.end());

    // Write own log
    while (true)
    {
        size_t pos = log.find('\n');
        if (pos == string::npos && last_call && !log.empty())
            pos = log.size();

        if (pos != string::npos)
        {
            log_callback(log.substr(0, pos));
            log.erase(0, pos + 1);
        }
        else
        {
            break;
        }
    }
}

static int SystemCall(string command, std::function<void(const string&)> log_callback)
{
#if defined(FO_WINDOWS) && !defined(WINRT)
    HANDLE out_read = nullptr;
    HANDLE out_write = nullptr;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;
    if (!CreatePipe(&out_read, &out_write, &sa, 0))
        return -1;
    if (!SetHandleInformation(out_read, HANDLE_FLAG_INHERIT, 0))
    {
        CloseHandle(out_read);
        CloseHandle(out_write);
        return -1;
    }

    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.hStdError = out_write;
    si.hStdOutput = out_write;
    si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    wchar_t* cmd_line = _wcsdup(_str(command).toWideChar().c_str());
    BOOL result = CreateProcessW(nullptr, cmd_line, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi);
    SAFEDELA(cmd_line);
    if (!result)
    {
        CloseHandle(out_read);
        CloseHandle(out_write);
        return -1;
    }

    string log;
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        DWORD bytes;
        while (PeekNamedPipe(out_read, nullptr, 0, nullptr, &bytes, nullptr) && bytes > 0)
        {
            char buf[TEMP_BUF_SIZE];
            if (ReadFile(out_read, buf, sizeof(buf), &bytes, nullptr))
            {
                log.append(buf, bytes);
                PrintLog(log, false, log_callback);
            }
        }

        if (WaitForSingleObject(pi.hProcess, 0) != WAIT_TIMEOUT)
            break;
    }
    PrintLog(log, true, log_callback);

    DWORD retval;
    GetExitCodeProcess(pi.hProcess, &retval);
    CloseHandle(out_read);
    CloseHandle(out_write);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return (int)retval;

#elif !defined(FO_WINDOWS) && !defined(FO_WEB)
    FILE* in = popen(command.c_str(), "r");
    if (!in)
        return -1;

    string log;
    char buf[TEMP_BUF_SIZE];
    while (fgets(buf, sizeof(buf), in))
    {
        log += buf;
        PrintLog(log, false, log_callback);
    }
    PrintLog(log, true, log_callback);

    return pclose(in);
#else
    return 1;
#endif
}

static int Global_SystemCall(string command)
{
    string prefix = command.substr(0, command.find(' '));
    return SystemCall(command, [&prefix](const string& line) { WriteLog("{} : {}\n", prefix, line); });
}

static int Global_SystemCallExt(string command, string& output)
{
    output = "";
    return SystemCall(command, [&output](const string& line) {
        if (!output.empty())
            output += "\n";
        output += line;
    });
}

static void Global_OpenLink(string link)
{
#if defined(FO_WINDOWS)
#ifndef WINRT
    ::ShellExecuteW(nullptr, L"open", _str(link).toWideChar().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#endif
#elif !defined(FO_IOS)
    int r = system((string("xdg-open ") + link).c_str());
    UNUSED_VARIABLE(r); // Supress compiler warning
#endif
}

static void* Global_GetProtoItem(hash pid, CScriptDict* props)
{
#if 0
    ProtoItem* proto = ProtoMngr.GetProtoItem( pid );
    if( !proto )
        return nullptr;

    Item* item = new Item( 0, proto );
    if( props )
    {
        for( asUINT i = 0; i < props->GetSize(); i++ )
        {
            if( !Properties::SetValueAsIntProps( (Properties*) &item->Props, *(int*) props->GetKey( i ), *(int*) props->GetValue( i ) ) )
            {
                item->Release();
                return nullptr;
            }
        }
    }
    return item;
#endif
    return 0;
}

static uint Global_GetUnixTime()
{
    return (uint)time(nullptr);
}
