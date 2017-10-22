##cmpxchg Test##

Built for testing the behavior of the Intel 64/IA32 `cmpxchg` and more specifically, the GCC `__sync_bool_compare_and_swap` function. The purpose of this application was to investigate if a spinlock on the result of `__sync_bool_compare_and_swap` is required until it returns true.

`__sync_bool_compare_and_swap` returns a bool to indicate if the swap successfully occured. This application proves that the swap itself doesn't occur if the function returns false. Compare with the `__sync_fetch_and_add` function (a `lock` followed by `xadd`) which will always carry out the addition atomically.

This is useful to know, because `__sync_bool_compare_and_swap` is often used to atomically increase the write position of a circular buffer from different threads, where a simple increment won't suffice.

NOTE: I'm still not entirely sure if this is the best compare/swap atomically. If there's anyone out there who is aware of a better way please get in touch! When I get some more time I would like to figure out how/if it is possible to make a `sync_increment_wrap` like function using 64/IA32 instructions, specifically for use with ring buffers.

---

Could try this:
TODO: Would like to try this with a wrap:

    u32 NewWritePosition = AtomicIncrement32(&State->EventsWritePosition);
    Result = State->Events + ((NewWritePosition-1) & (DEBUG_EVENTS_COUNT-1));

