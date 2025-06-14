#pragma once

#include <chrono>
#include <stdio.h>
#include <thread>
#include <sys/types.h>

using PERIOD = std::nano;
using CLK = std::chrono::system_clock;
using DUR = std::chrono::duration<__int128_t, PERIOD>;
using TP = std::chrono::time_point<CLK, DUR>;

template
        <
            auto Func = 0
        >
struct MakeFunc
{
    typedef decltype(Func) type;
    constexpr const static type value = Func;
};

template
        <
            typename callback,
            auto iterations = 100,
            typename ITER_T = decltype(iterations),
            typename CLK_T = CLK,
            typename DUR_T = std::chrono::duration<ITER_T, PERIOD>,
            typename TP_T = std::chrono::time_point<CLK_T, DUR_T>
        >
DUR_T
doBenchmark
        () 
{
    DUR_T sum = DUR_T(0);
    CLK::time_point start, temp;
        
    start = CLK_T::now();

    for (
        decltype(iterations) i = 0;
        i < iterations;
        ++i
    )
    {
        callback::value();

        temp = start;
        start = CLK_T::now();
        sum += std::chrono::duration_cast<DUR_T>(start - temp);
    }

    return sum;
}

template
        <
            typename T
        >
struct ToString
{
    static constexpr const char* value = "und";
};

template <> struct ToString<std::mega> { static constexpr const char* value = "Ms"; };
template <> struct ToString<std::kilo> { static constexpr const char* value = "ks"; };
template <> struct ToString<std::ratio<1>> { static constexpr const char* value = "s"; };
template <> struct ToString<std::milli> { static constexpr const char* value = "ms"; };
template <> struct ToString<std::micro> { static constexpr const char* value = "μs"; };
template <> struct ToString<std::nano> { static constexpr const char* value = "ns"; };
template <> struct ToString<std::pico> { static constexpr const char* value = "ps"; };

template
        <
            typename callback,
            auto iterations,
            typename ITER_T = decltype(iterations)
        >
inline void
printBench
        (
            const char *name,
            FILE *output
        )
{
    using DUR_T = std::chrono::duration<__int128_t, std::micro>;
    auto total_time = doBenchmark<callback, iterations, ITER_T, CLK, DUR_T>();
    ITER_T avg = total_time.count() / __int128_t(iterations);
    fprintf(output, "%s: iterations %i average time %i%s\n", name, iterations, avg, ToString<typename DUR_T::period>::value);
}

template
        <
            auto time,
            typename PERIOD_T = std::nano,
            typename DUR_T = std::chrono::duration<int64_t, PERIOD_T>
        >
inline void
testBench
        ()
{
    TP start = CLK::now();
    std::this_thread::sleep_until(start + DUR_T(time));
}

/*
int 
main
        ()
{
    printBench<MakeFunc<testBench<1, std::micro>>, 10>("test_bench", stderr);
    printBench<MakeFunc<testBench<10, std::micro>>, 10>("test_bench", stderr);
    printBench<MakeFunc<testBench<100, std::micro>>, 10>("test_bench", stderr);
    printBench<MakeFunc<testBench<1000, std::micro>>, 10>("test_bench", stderr);
    printBench<MakeFunc<testBench<10000, std::micro>>, 10>("test_bench", stderr);

    return 0;
}
*/