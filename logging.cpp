#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <tuple>
#include <iostream>
#include <string>
#include <format>

#define LOG_BUFFER_SIZE 100

template<typename IType, typename FType = float, typename RType = IType>
constexpr inline RType lerp(const IType &v1, const IType &v2, const FType &factor) {
    return RType(RType(v1) * (FType(1) - factor) + RType(v2) * factor);
}

template<typename TIME_T = int64_t>
struct DataPointBaseT {
    using time_type = TIME_T;

    TIME_T time;

    constexpr DataPointBaseT(const TIME_T &time):time(time) {}
    constexpr DataPointBaseT():DataPointBaseT(0) {}

    constexpr inline TIME_T get_time() const { return time; }

    template<typename RType=float>
    constexpr inline RType get_factor(const DataPointBaseT &other, const time_type &time) {
        return RType(time - this->time) / RType(other.time - this->time);
    }
};

template<typename TIME_T = int64_t, typename POINT_T = unsigned short, typename DataPointBase = DataPointBaseT<TIME_T>>
struct DataPointT : public DataPointBase {
    using time_type = typename DataPointBase::time_type;
    using value_type = POINT_T;

    POINT_T value;

    constexpr DataPointT(const TIME_T &time, const POINT_T &value):DataPointBase(time),value(value){}
    constexpr DataPointT():DataPointT(0,0){}

    constexpr inline POINT_T get_value() const { return value; }

    template<typename FType=float>
    constexpr inline DataPointT interpolate(const DataPointT &other, const FType &factor) const {
        return DataPointT(
            lerp(this->time, other.time, factor),
            lerp(value, other.value, factor)
        );
    }
};

template<typename ...Args>
struct DataValueTupleT {
    using DPTup = DataValueTupleT<Args...>;
    
    std::tuple<Args...> members;

    constexpr DataValueTupleT(Args ...args):members(std::make_tuple(args...)){}

    template<typename Op, size_t ...Is>
    static DPTup apply_op(const DPTup &lhs, const DPTup &rhs, Op op, std::index_sequence<Is...>) {
        return DPTup(op(std::get<Is>(lhs.members), std::get<Is>(rhs.members))...);
    }

    template<typename RHSType, typename Op, size_t ...Is>
    static DPTup apply_op(const DPTup &lhs, const RHSType &rhs, Op op, std::index_sequence<Is...>) {
        return DPTup(op(std::get<Is>(lhs.members), rhs)...);
    }

    template<typename OtherT = DPTup>
    DPTup operator*(const OtherT &other) {
        return apply_op(*this, other, [](auto a, auto b){return a * b;}, std::index_sequence_for<Args...>{});
    }

    template<typename OtherT = DPTup>
    DPTup operator+(const OtherT &other) {
        return apply_op(*this, other, [](auto a, auto b){return a + b;}, std::index_sequence_for<Args...>{});
    }
};

template<typename Derived>
struct DataPointImplT : public Derived {
    using time_type = typename Derived::time_type;
    using value_type = typename Derived::value_type;
    using dp_type = DataPointImplT<Derived>;

    using Derived::Derived;

    constexpr inline value_type get_value() const { return Derived::value; }

    template<typename FType=float>
    constexpr inline dp_type interpolate(const dp_type &other, const FType &factor) const {
        return dp_type(
            lerp(this->time, other.time, factor),
            lerp(this->value, other.value, factor)
        );
    }
};

template<typename TIME_T = int64_t, typename DataPointBase = DataPointBaseT<TIME_T>>
struct DPLTR390STBase : public DataPointBase {
    using DVT = DataValueTupleT<uint32_t, uint32_t, float, float>;
    using time_type = typename DataPointBase::time_type;
    using value_type = DVT;

    union {
        DVT value;
        struct {
            float uvi, lux;
            uint32_t uvs, als;
        };
    };

    constexpr DPLTR390STBase(const time_type &time, const uint32_t &als, const uint32_t &uvs, const float &lux, const float &uvi)
        :DataPointBase(time),als(als),uvs(uvs),lux(lux),uvi(uvi){}
    constexpr DPLTR390STBase(const time_type &time, const DVT &value):DataPointBase(time),value(value){}

    std::string to_string() {
        return std::format("ALS {} UVS {} Lux {} UVI {}", als, uvs, lux, uvi);
    }
};

using DPLTR390ST = DataPointImplT<DPLTR390STBase<>>;

template<typename T, int LOOP_SIZE = LOG_BUFFER_SIZE>
struct LoopBufferT {
    static constexpr const int _size = LOOP_SIZE;

    int index, count;
    T data[_size];

    constexpr LoopBufferT():index(0),count(0){}

    constexpr inline int size() const { return count; }

    constexpr inline int capacity() const { return _size; }

    constexpr inline void clear() { count = 0; index = 0; }

    constexpr inline void push_back(const T &value) {
        if (index >= _size)
            index = 0;
        if (count < _size)
            count++;
        
        data[index++] = value;
    }

    constexpr inline int to_rel(const int &pos) const {
        int i = pos + index;

        if (i < 0) i = count - i;

        while (i + 1 > count) i -= count;

        return i;
    }

    constexpr inline T &get(const int &pos) {
        return data[to_rel(pos)];
    }

    constexpr inline bool has(const int &pos) const {
        return count && to_rel(pos) >= 0;
    }
};

template<typename DataPoint = DataPointT<int, unsigned short>, typename DataStorage = LoopBufferT<DataPoint>>
struct DataLogT {
    using point_type = DataPoint;
    using time_type = typename DataPoint::time_type;
    using value_type = typename DataPoint::value_type;
    using storage_type = DataStorage;

    DataStorage &log;
    int64_t time_start;

    constexpr DataLogT(DataLogT &data_log):log(data_log.log),time_start(data_log.time_start){}
    constexpr DataLogT(DataStorage &log):log(log),time_start(0){}
    constexpr DataLogT(DataStorage &log, const int64_t &time_start):log(log),time_start(time_start){}
    
    constexpr inline void set_start_time(const int64_t &time) { time_start = time; }

    constexpr inline void set_log(DataStorage &log) { this->log = log; }

    constexpr inline void set_log(DataStorage *log) { set_log(*log); }

    constexpr inline int64_t get_start_time() const { return time_start; }

    constexpr inline int64_t get_data_time(const time_type &time) const { return time_start + time; }

    constexpr inline int64_t get_data_time(const point_type &point) const { return time_start + point.time; }

    constexpr inline int64_t get_data_index_time(const int &index) const { return get_data_time(has(index) ? get(index).time : 0); }

    constexpr inline int64_t get_data_start_time() const { return get_data_index_time(0); }

    constexpr inline int64_t get_data_end_time() const { return get_data_index_time(-1); }

    constexpr inline time_type get_data_range_time() const { return time_type(get_data_end_time() - get_data_start_time()); }

    constexpr inline int size() const { return log.template size(); }

    constexpr inline int capacity() const { return log.template capacity(); }

    constexpr inline void clear() { log.template clear(); }

    constexpr inline void push_back(const DataPoint &point) { log.template push_back(point); }

    constexpr inline void push_back(const time_type &time, const value_type &value) {
        push_back(point_type(time - time_start, value));
    }

    constexpr inline bool has(const int &pos) const { return log.template has(pos); }

    constexpr inline DataPoint &get(const int &pos) { return log.template get(pos); }

    constexpr inline const DataPoint &get(const int &pos) const { return log.template get(pos); }

    /*
        Returns the previous nearest value or the exact match, never the upper bound
    */
    constexpr int binary_index(const time_type &time, const int &start, const int &end, const int &depth=0) const {
        assert(depth < 32 && "Too much recursion");

        const int range = end - start;

        if (range < 2)
            return start;

        const int mid = start + (range/2);
        const point_type &middle = get(mid);

        if (middle.time <= time)
            return binary_index(time, mid, end, depth+1);
        else
            return binary_index(time, start, mid, depth+1);
    }

    constexpr inline int binary_index(const time_type &time) const {
        return binary_index(time, 0, size());
    }

    constexpr inline point_type &binary_search(const time_type &time) {
        return get(binary_index(time));
    }

    constexpr inline bool time_pair(const time_type &time, int &first, int &second) const {
        int i = binary_index(time);

        if (i < 1) {
            if (!size())
                return false;
            first = 0;
            second = 1;
            return true;
        }

        if (i >= size() - 1) {
            first = size()-2;
            second = size()-1;
            return true;
        }

        first = i;
        second = i+1;
        return true;
    }

    constexpr inline bool time_pair(const time_type &time, point_type *first, point_type *second) {
        int a, b;

        if (!time_pair(time, a, b))
            return false;
        
        *first = get(a);
        *second = get(b);

        return true;
    }

    constexpr inline bool time_pair(const time_type &time, point_type &first, point_type &second) {
        return time_pair(time, &first, &second);
    }

    constexpr inline point_type interpolate_point(const time_type &time) {
        point_type v1, v2;

        if (!time_pair(time, v1, v2))
            return v1;

        float factor = float(time - v1.time) / float(v2.time - v1.time);

        return v1.interpolate(v2, factor);
    }

    template<typename RType = value_type>
    constexpr inline RType interpolate_value(const time_type &time) {
        point_type v1, v2;

        if (!time_pair(time, v1, v2))
            return RType(0);

        float factor = v1.get_factor(v2, time);

        return lerp<value_type, float, RType>(v1.value, v2.value, factor);
    }

    constexpr inline value_type min() const {
        if (!size())
            return 0;

        value_type v = get(0).value;

        for (int i = 1; i < size(); i++) {
            const value_type &p = get(i).value;
            if (p < v)
                v = p;
        }

        return v;
    }

    constexpr inline value_type max() const {
        if (!size())
            return 0;

        value_type v = get(0).value;

        for (int i = 1; i < size(); i++) {
            const value_type &p = get(i).value;
            if (p > v)
                v = p;
        }

        return v;
    }

    constexpr inline value_type range() const {
        return max() - min();
    }

    template<typename RType = int64_t>
    constexpr inline int64_t sum() const {
        RType s = RType(0);

        for (int i = 0; i < size(); s += get(i).value, i++);
        
        return s;
    }

    template<typename RType = value_type>
    constexpr inline RType avg() const {
        if (!size())
            return RType(0);

        return
            this->sum<RType>() / RType(size());
    }

    template<typename RType = int64_t>
    constexpr inline int64_t sum_range(const int &start_index, const int &end_index) const {
        RType s = RType(0);

        for (int i = start_index; i < size() && i < end_index; s += get(i).value, i++);

        return s;
    }

    template<typename RType = int64_t>
    constexpr inline int64_t sum_range_time(const time_type &start, const time_type &end) const {
        return sum_range<RType>(binary_index(start), binary_index(end));
    }

    template<typename RType = value_type>
    constexpr inline RType avg_range(int start_index, int end_index) const {
        const int range = end_index - start_index;
        if (!range || !size())
            return RType(0);

        return
            this->sum_range<RType>(start_index, end_index) / RType(range);
    }

    template<typename RType = value_type>
    constexpr inline RType avg_range_time(const time_type &start, const time_type &end) const {
        return avg_range<RType>(binary_index(start), binary_index(end));
    }
};


int main() {
    using DP = DataPointT<int, int>;
    using LB = LoopBufferT<DP, 15>;
    using DL = DataLogT<DP, LB>;

    LB buffer;
    DL log(buffer);

    for (int i = 0; i < 20; i++) {
        log.push_back({i*1000,(int)(sinf(i*0.5f)*5.0f)});
        printf("%i, %i, %i, %i, %i\n", i, log.log.index, log.log.size(), log.log.capacity(), log.log._size);
    }

    for (int i = 0; i < log.size(); i++) {
        DP &n = log.get(i);
        printf("log: %i, %i, %i\n", i, n.time, n.value);
    }

    int s = 0;
    DP &n = log.binary_search(s);
    for (int i = s; i<log.size(); i++) {
         n = log.binary_search(i*1000-1);
        printf("search: %i, %i, %i\n", i, n.time, n.value);
    }

    for (float i = 6000.0f; i < 7000.0f; i+=100.0f) {
        DP v = log.interpolate_point(i);
        printf("interpolate: %f, %i, %f\n", i, v.time, log.interpolate_value<float>(i));
    }

    DPLTR390ST dp1(9, 324, 58583, 0.55, 5.55);
    DPLTR390ST dp2(60, 900, 423, 6.9, 0.3);
    DPLTR390ST dp3 = dp1.interpolate(dp2, 0.5);
    std::cout << dp1.to_string() << std::endl;
    std::cout << dp2.to_string() << std::endl;
    std::cout << dp3.to_string() << std::endl;

    return 0;
}