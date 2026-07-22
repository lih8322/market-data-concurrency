#pragma once

//==============================================================================
//
//   출처 : 실시간 시세 수집/분석 앱 — domain/model/StockHot.h
//
//   역할 : 종목별 실시간 '핫' 상태. 자주 바뀌는 값만 모아 두고, 종목명·시장구분
//          같은 '콜드' 메타는 StockRegistry 가 따로 보관한다(핫/콜드 분리).
//          [4] consumer 스레드가 갱신 → Snapshot 에 담겨 UI 다수 스레드가 조회
//
//==============================================================================

#include "Symbol.h"
#include "Tick.h"   
#include <cstdint>

namespace domain {

struct alignas(64) StockHot {
    Symbol      symbol{};
    TimestampNs ts_ns{};

    Price       open{};
    Price       high{};
    Price       low{};
    Price       close{};
    Price       prev_close{};

    Notional    trd_value{};      // 누적 거래대금 (원)
    int64_t     net_pvalue{};     // 프로그램 순매수 금액 (원, 음수 가능)

    bool        limit_up{false};     // 상한 여부 (대비부호 '1')
};

}
