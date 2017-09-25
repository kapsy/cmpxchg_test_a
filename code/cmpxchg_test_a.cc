#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dispatch/dispatch.h>

#define Assert(Expression) if(!(Expression)) {abort();}
#define rdtsc __builtin_readcyclecounter

#define BitsPerByte 8
#define BITS_PER_BYTE 8
#define SizeOfFloat sizeof(float)

#define NSPerUS (1000.0)
#define NSPerMS (1000.0 * NSPerUS)
#define NSPerS  (1000.0 * NSPerMS)

#define USPerMS (1000.0)
#define USPerS  (1000.0 * USPerMS)

// TODO: (KAPSY) Just use these guys!
#define SToUS(Value) ((Value) * USPerS)
#define SToNS(Value) (Value * NSPerS)

#define MSToS(Value)
#define MSToUS(Value) ((Value) * USPerMS)
#define MSToNS(Value) (Value * NSPerMS)

#define USToS(Value)
#define USToNS(Value)

#define NSToS(Value) ((Value) / NSPerS)
#define NSToMS(Value) ((Value) / NSPerMS)
#define NSToUS(Value) ((Value) / NSPerUS)

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef size_t memory_index;
typedef float real32;
typedef double real64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef int32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float r32;
typedef double r64;

#define Real32Maximum FLT_MAX

typedef size_t size;
typedef intptr_t intptr;
typedef uintptr_t uintptr;
typedef uintptr_t umm;

#define internal static
#define local_persist static
#define global_variable static

#define elif else if
#define Pi32 3.14159265359f
#define Tau32 6.28318530717959f

static u32 GlobalSharedValue;
static u32 GlobalThreadCount;

inline b32
AtomicCompareExchangeU32_B32(u32 volatile *TheValue, u32 OldValue, u32 NewValue)
{
    b32 Result = __sync_bool_compare_and_swap(TheValue, OldValue, NewValue);
    return(Result);
}

inline u32
AtomicAddU32_U32(u32 volatile *TheValue, u32 Addend)
{
    // NOTE: (Kapsy) Returns the original value, prior to adding.
    u32 Result = __sync_fetch_and_add(TheValue, Addend);
    return(Result);
}

#define MAX_COUNTS (1 << 16)

static void
DoExchangesUsingAdd(u32 ThreadIndex)
{
    long long StartCount = rdtsc();

    u32 Count = 0;
    while(Count++ < MAX_COUNTS)
    {
        AtomicAddU32_U32(&GlobalSharedValue, 1);
    }
    long long EndCount = rdtsc();
    float MegaCycles = (float)((EndCount - StartCount) / (1000.f * 1000.f));

    printf("Thread %d finished.\nCount:%d\nMegaCycles:%.2f\n\n",
            ThreadIndex, Count, MegaCycles);
}

static void
DoExchanges(u32 ThreadIndex)
{
    long long StartCount = rdtsc();

    u32 Count = 0;
    u32 SuccessfulExchanges = 0;
    u32 TotalExchanges = 0;
    while(Count++ < MAX_COUNTS)
    {
#if 1
        u32 CurrentValue, NewValue;
        do
        {
            CurrentValue = GlobalSharedValue;
            NewValue = CurrentValue+1;
            ++TotalExchanges;
        }
        while(!AtomicCompareExchangeU32_B32(&GlobalSharedValue, CurrentValue, NewValue));
#else
        GlobalSharedValue += 1;
#endif

    }
    long long EndCount = rdtsc();
    float MegaCycles = (float)((EndCount - StartCount) / (1000.f * 1000.f));

    printf("Thread %d finished.\nCount:%d\nTotalExchanges:%d\nMegaCycles:%.2f\n\n",
            ThreadIndex, Count, TotalExchanges, MegaCycles);
}


static dispatch_semaphore_t SemaphoreHandle;

static void *
AlternateThread(void *Payload)
{
    u32 ThreadIndex = *((u32 *)Payload);
#if 1
    DoExchanges(ThreadIndex);
#else
    DoExchangesUsingAdd(ThreadIndex);
#endif
    dispatch_semaphore_signal(SemaphoreHandle);
    return(0);
}

int main(int argc, char **argv)
{
    long long StartCount = rdtsc();

    u32 InitialCount = 0;
    SemaphoreHandle = dispatch_semaphore_create(InitialCount);

    int Result = 0;

    u32 ThreadIndex0 = 0;
    pthread_t Thread0;
    Result = pthread_create(&Thread0, 0, &AlternateThread,
            (void *)&ThreadIndex0);
    Assert(Result == 0);

    u32 ThreadIndex1 = 1;
    pthread_t Thread1;
    Result = pthread_create(&Thread1, 0, &AlternateThread,
            (void *)&ThreadIndex1);
    Assert(Result == 0);

    dispatch_semaphore_wait(SemaphoreHandle, DISPATCH_TIME_FOREVER);
    dispatch_semaphore_wait(SemaphoreHandle, DISPATCH_TIME_FOREVER);

    long long EndCount = rdtsc();
    float MegaCycles = (float)((EndCount - StartCount) / (1000.f * 1000.f));

    printf("All threads finished.\nGlobalSharedValue:%d\nMegaCycles:%.2f\n",
            GlobalSharedValue, MegaCycles);

    return(EXIT_SUCCESS);
}
