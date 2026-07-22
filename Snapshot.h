#pragma once

//==============================================================================
//
//   출처 : 실시간 시세 수집/분석 앱 — common/concurrency/Snapshot.h
//
//   역할 : 단일 writer / 다수 reader 를 락 없이 잇는 더블 버퍼 프리미티브.
//          [3] consumer 스레드가 update() → UI 스레드들이 get() 으로 조회
//
//==============================================================================

#include <array>
#include <atomic>
#include <cstdint>

// 범용 lock-free 더블 버퍼 스냅샷 프리미티브.
//   - 비즈니스 의미 없음(순수 동시성 메커니즘) → domain 이 아니라 common 에 둔다.
//   - 단일 writer / 다수 reader 전제.
//   - T 는 복사 가능해야 한다.
//
// writer 는 비활성 버퍼에 기록한 뒤 activeIndex_ 를 release 로 flip,
// reader 는 acquire 로 activeIndex_ 를 읽어 해당 버퍼를 값 복사한다.
// 따라서 reader 는 항상 찢어지지 않은(torn-free) 일관 스냅샷을 받는다.

namespace common {

template<typename T>
class Snapshot {
public:
    Snapshot() { active_.store(0, std::memory_order_relaxed); }

    Snapshot(const Snapshot&)            = delete;
    Snapshot& operator=(const Snapshot&) = delete;

    Snapshot(Snapshot&& other) noexcept {
        buffers_ = std::move(other.buffers_);
        active_.store(other.active_.load(std::memory_order_relaxed),
                      std::memory_order_relaxed);
    }
    Snapshot& operator=(Snapshot&& other) noexcept {
        if (this != &other) {
            buffers_ = std::move(other.buffers_);
            active_.store(other.active_.load(std::memory_order_relaxed),
                          std::memory_order_relaxed);
        }
        return *this;
    }

    // 단일 writer 에서 호출
    void update(const T& data) {
        uint32_t w = 1u - active_.load(std::memory_order_relaxed);
        buffers_[w] = data;                              // 비활성 버퍼에 기록
        active_.store(w, std::memory_order_release);     // flip
    }

    // 다수 reader 에서 lock 없이 호출 가능
    [[nodiscard]] T get() const {
        uint32_t r = active_.load(std::memory_order_acquire);
        return buffers_[r];                              // 값 복사 (일관 스냅샷)
    }

private:
    std::array<T, 2>      buffers_{};
    std::atomic<uint32_t> active_;
};

}
