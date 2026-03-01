#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
using Socket = SOCKET;
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using Socket = int;
#endif

namespace Net {
// --- Init / Shutdown ---

inline bool Init() {
#ifdef _WIN32
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
#else
    return true;
#endif
}

inline void Shutdown() {
#ifdef _WIN32
    WSACleanup();
#endif
}

// --- Close socket ---

inline void Close(Socket s) {
#ifdef _WIN32
    closesocket(s);
#else
    close(s);
#endif
}

// --- Set non-blocking ---

inline bool SetNonBlocking(Socket s) {
#ifdef _WIN32
    u_long mode = 1;
    return ioctlsocket(s, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(s, F_GETFL, 0);
    return fcntl(s, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
}
} // namespace Net
