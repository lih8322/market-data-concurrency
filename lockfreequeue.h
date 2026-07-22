#pragma once

//==============================================================================
//
//   출처 : Anthony Williams, "C++ Concurrency in Action" (Manning)
//          Chapter 7 — Designing lock-free concurrent data structures
//          Listing "A single-producer, single-consumer lock-free queue"
//          학습 목적의 발췌 코드로, 직접 설계한 자료구조가 아니다.
//
//   한계 : 원소마다 동적 할당이 발생한다.
//          push 는 new node + make_shared 로 2회, pop 은 delete 로 회수한다.
//          소비자가 밀리면 메모리가 무한히 늘어난다(백프레셔 부재).
//
//   결정 : 고정 크기 ring buffer 방식인 boost::lockfree::spsc_queue 를 채택.
//          capacity<N> 으로 저장소를 멤버 배열에 두어 구동 후 할당이 0 이고,
//          연속 메모리라 캐시 지역성이 좋으며, 가득 차면 push 가 실패해
//          백프레셔가 자연히 걸린다. (현재 프로젝트에 적용)
//==============================================================================

#ifndef LOCKFREEQUEUE
#define LOCKFREEQUEUE
#include <atomic>
#include <memory>

template <typename T>
class LockFreeQueue {
private:
    struct node {
        std::shared_ptr<T> data;
        node* next;
        node() : next(nullptr) {}
    };
    std::atomic<node*> head;
    std::atomic<node*> tail;
    node* pop_head() {
        node* const old_head = head.load();
        if (old_head == tail.load()) {
            return nullptr;
        }
        head.store(old_head->next);
        return old_head;
    }

public:
    LockFreeQueue() : head(new node), tail(head.load()) {}
    LockFreeQueue(const LockFreeQueue& other) = delete;
    LockFreeQueue& operator=(const LockFreeQueue& other) = delete;
    ~LockFreeQueue() {
        while (node* const old_head = head.load()) {
            head.store(old_head->next);
            delete old_head;
        }
    }
    std::shared_ptr<T> pop() {
        node* old_head = pop_head();
        if (!old_head) {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> const res(old_head->data);
        delete old_head;
        return res;
    }

    void push(T new_value) {
        std::shared_ptr<T> new_data(std::make_shared<T>(new_value));
        node* p = new node;
        node* const old_tail = tail.load();
        old_tail->data.swap(new_data);
        old_tail->next = p;
        tail.store(p);
    }
};
#endif // !LOCKFREEQUEUE