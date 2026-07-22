#pragma once

//==============================================================================
//
//   출처 : 실시간 시세 수집/분석 앱 — domain/model/Tick.h
//
//   역할 : Cybos/KIS 등 벤더별 원시 틱을 하나로 정규화한 도메인 모델.
//          [2] 벤더 어댑터 → NormalizedTick → SPSC 큐 → consumer 스레드
//
//==============================================================================

#include "Symbol.h"
#include <cstdint>

namespace domain {

using Price       = int32_t;
using Quantity    = int64_t;
using Notional    = int64_t;
using TimestampNs = int64_t;

// 체결상태 — 체결가가 매도호가에 닿았는지 매수호가에 닿았는지.
enum class ExecSide : uint8_t {
    Unknown = 0,   // 미수신(배치 조회 경로 등)
    Buy     = 1,   // 매수 체결
    Sell    = 2,   // 매도 체결
};

struct alignas(64) NormalizedTick {
    Symbol      symbol{};
    TimestampNs ts_ns{};
    Quantity    last_qty{};              // 순간체결수량
    Notional    cumulative_notional{};   // 누적 거래대금 (원)

    Price       open{};
    Price       high{};
    Price       low{};
    Price       close{};
    Price       prev_close{};

    bool        limit_up{false};   // 상한 여부 (대비부호 '1' 에서 판정)
    ExecSide    exec_side{};
};

}
