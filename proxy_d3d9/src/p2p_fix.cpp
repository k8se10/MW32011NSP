// p2p_fix.cpp -- Finding 1: iw5sp.exe Steamworks P2P receive-path overflow.
//
// Root cause (full trail in re_notes/vulnerability_research.md and
// re_notes/INTERNAL_vulnerability_research.md's own x64-recompile-verification
// section): FUN_1402893f0 (x64 -- x86 was FUN_00511a20) polls the real, documented
// Steamworks SDK interface ISteamNetworking in a loop. It calls the real
// IsP2PPacketAvailable (vtable slot at interface+0x8 in this x64 build) to learn
// the REMOTE PEER's own self-reported pending-packet size, then passes that value
// DIRECTLY AND WITHOUT ANY BOUNDS CHECK as the destination-buffer-size (cubDest)
// argument to the real ReadP2PPacket (vtable slot at interface+0x10), which copies
// into a fixed 1200-byte stack buffer (local_4c8[1200] in the decompile). A
// malicious P2P peer in an active Spec-Ops/Survival co-op session can report an
// inflated size to overflow that buffer.
//
// FIX DESIGN, chosen specifically to avoid reimplementing FUN_1402893f0's own
// substantial post-read message-parsing logic (calls into several other functions
// this project doesn't have full RE detail on): hook ReadP2PPacket itself, at the
// real Steamworks vtable level (the standard, correct way to intercept a COM-style
// virtual method -- this project's own sibling MW32011NCP already uses the same
// vtable-hook technique for IDirect3DDevice9::EndScene/CreateDevice). ReadP2PPacket
// is presumably called from other, legitimate, unrelated places in the game's own
// netcode too -- a BLANKET clamp on every call would risk truncating some other
// caller's genuinely larger, valid packet. So the detour is SCOPED: it inspects its
// own return address (the address execution resumes at once ReadP2PPacket returns,
// i.e. the instruction immediately after the CALL) and only clamps cubDest when
// that return address falls within FUN_1402893f0's own real function body (verified
// via Ghidra: [0x1402893f0, 0x1402895b2), 0x1C2 bytes) -- every other caller
// anywhere else in the binary passes through completely untouched.
//
// The fix itself is a one-line correction, not a rewrite: pass min(cubDest, 1200)
// to the real ReadP2PPacket instead of the attacker-reported value. This dequeues
// the pending packet correctly (Steamworks' own documented ReadP2PPacket behavior
// for a too-small destination handles a legitimate size mismatch safely -- this is
// completely standard, correct API usage, not something usable maliciously) rather
// than either crashing (the original bug) or silently leaving the oversized packet
// stuck at the head of the peer's queue forever (which a naive "just skip it"
// fix would do, itself a real DoS risk against the local player's own P2P session).

#include "netcode_fixes.h"
#include "signature_scan.h"

#include <windows.h>
#include <cstdio>
#include <intrin.h>

namespace {

constexpr size_t kP2PFuncBodySize = 0x1C2; // real, Ghidra-confirmed function body size

// Real, fixed 52-byte prefix of FUN_1402893f0 -- confirmed via disassembly to be
// entirely free of PC-relative/absolute address references (every instruction in
// this range is either a fixed opcode or an RSP/RBP-relative stack displacement,
// neither of which needs wildcarding -- see signature_scan.h's own header comment
// for why a stack-relative displacement is not an "address" in the sense that
// matters here). No wildcards needed at all for this signature.
constexpr char kP2PFuncSignature[] =
    "40 55 56 48 8D AC 24 88 FB FF FF 48 81 EC 78 05 00 00 81 A5 A4 04 00 00 FF FF 0F FF "
    "33 F6 40 88 B5 A7 04 00 00 81 A5 A4 04 00 00 00 00 F0 FF 89 B5 A0 04 00 00";

constexpr size_t kRealDestBufferSize = 1200; // local_4c8[1200] in the decompile -- the
    // REAL, confirmed destination buffer size FUN_1402893f0's own stack frame reserves

uintptr_t g_p2pFuncStart = 0;

// Real Steamworks SDK signature (publicly documented, not game-specific):
//   bool ReadP2PPacket(void *pubDest, uint32 cubDest, uint32 *pcubMsgSize,
//                       CSteamID *psteamIDRemote, int nChannel = 0);
// Confirmed via this project's own decompile of the vulnerable call site that this
// x64 build's calling convention matches a plain __fastcall (this*, dest, cubDest,
// pcubMsgSize, psteamIDRemote, nChannel) -- standard x64 Steamworks ABI.
using ReadP2PPacket_t = bool(__fastcall*)(void* thisPtr, void* pubDest, uint32_t cubDest,
                                            uint32_t* pcubMsgSize, void* psteamIDRemote, int nChannel);
ReadP2PPacket_t g_realReadP2PPacket = nullptr;

bool __fastcall Hook_ReadP2PPacket(void* thisPtr, void* pubDest, uint32_t cubDest,
                                     uint32_t* pcubMsgSize, void* psteamIDRemote, int nChannel)
{
    // _ReturnAddress() -- the address execution resumes at once this detour
    // returns, i.e. the instruction immediately following the CALL that reached
    // us. Scoping on this (rather than clamping every ReadP2PPacket call
    // globally) is what keeps this fix from affecting any other, unrelated,
    // legitimate caller of the same real Steamworks interface method.
    uintptr_t returnAddr = reinterpret_cast<uintptr_t>(_ReturnAddress());
    if (g_p2pFuncStart != 0 &&
        returnAddr >= g_p2pFuncStart && returnAddr < g_p2pFuncStart + kP2PFuncBodySize) {
        if (cubDest > kRealDestBufferSize) {
            cubDest = static_cast<uint32_t>(kRealDestBufferSize);
        }
    }
    return g_realReadP2PPacket(thisPtr, pubDest, cubDest, pcubMsgSize, psteamIDRemote, nChannel);
}

} // namespace

void InstallP2PFix(const FixHostServices& host)
{
    void* moduleBase = host.GetGameModuleBase();
    if (!moduleBase) {
        host.Log("[nsp-p2p-fix] FAILED: no game module base -- cannot resolve FUN_1402893f0");
        return;
    }

    // Only iw5sp.exe (Campaign/Survival/Spec-Ops) has this vulnerable function --
    // a failed scan on iw5mp.exe is EXPECTED, not an error, since this DLL may be
    // loaded into either binary. Log at info level, not FATAL.
    auto dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(moduleBase);
    auto ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS64*>(
        reinterpret_cast<uintptr_t>(moduleBase) + dosHeader->e_lfanew);
    size_t moduleSize = ntHeaders->OptionalHeader.SizeOfImage;

    SigScan::Result funcResult = SigScan::FindPattern(
        kP2PFuncSignature, reinterpret_cast<uintptr_t>(moduleBase), moduleSize);
    if (!funcResult.found) {
        host.Log("[nsp-p2p-fix] Signature not found -- expected if this is iw5mp.exe, "
                 "not a fix failure. Skipping (Finding 1 only affects iw5sp.exe).");
        return;
    }
    g_p2pFuncStart = funcResult.address;

    // Resolve the REAL Steamworks SDK export directly -- SteamNetworking() is a
    // genuine, publicly-documented export of steam_api64.dll, confirmed via this
    // project's own Ghidra symbol lookup to be a real EXTERNAL (not internal game
    // code) symbol. Calling it ourselves, exactly the way the game's own code
    // does, is standard, correct Steamworks SDK usage -- not reading or writing
    // any gameplay-entity memory, just the same public API surface any legitimate
    // Steamworks-integrated tool already uses.
    HMODULE steamApi = GetModuleHandleA("steam_api64.dll");
    if (!steamApi) {
        host.Log("[nsp-p2p-fix] FAILED: steam_api64.dll not loaded yet -- Steamworks "
                 "may not be initialized this early. Fix not installed this pass.");
        return;
    }
    using SteamNetworking_t = void*(__fastcall*)();
    auto steamNetworking = reinterpret_cast<SteamNetworking_t>(GetProcAddress(steamApi, "SteamNetworking"));
    if (!steamNetworking) {
        host.Log("[nsp-p2p-fix] FAILED: steam_api64.dll missing SteamNetworking export");
        return;
    }
    void* iface = steamNetworking();
    if (!iface) {
        host.Log("[nsp-p2p-fix] FAILED: SteamNetworking() returned null -- interface not ready yet");
        return;
    }

    // Real vtable layout confirmed via this project's own decompile of the
    // vulnerable call site: IsP2PPacketAvailable at interface+0x8, ReadP2PPacket
    // at interface+0x10 (both real Valve Steamworks SDK slots, matching what the
    // game's own code already dispatches through -- see this file's own header
    // comment for the exact decompiled call).
    void** vtable = *reinterpret_cast<void***>(iface);
    void* readP2PPacketAddr = vtable[2]; // +0x10 / sizeof(void*) = index 2

    void* original = nullptr;
    if (!host.InstallHook(readP2PPacketAddr, reinterpret_cast<void*>(&Hook_ReadP2PPacket), &original)) {
        char buf[256];
        sprintf_s(buf, "[nsp-p2p-fix] FAILED: InstallHook failed for ReadP2PPacket @ %p", readP2PPacketAddr);
        host.Log(buf);
        return;
    }
    g_realReadP2PPacket = reinterpret_cast<ReadP2PPacket_t>(original);

    char buf[256];
    sprintf_s(buf, "[nsp-p2p-fix] Installed. FUN_1402893f0 @ 0x%llX (body size 0x%zX), "
              "ReadP2PPacket @ %p hooked, scoped clamp to %zu bytes.",
              static_cast<unsigned long long>(g_p2pFuncStart), kP2PFuncBodySize,
              readP2PPacketAddr, kRealDestBufferSize);
    host.Log(buf);
}
