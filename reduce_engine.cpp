#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <functional>
#include <iterator>
#include <algorithm>
#include <format>
#include <memory>
#include <sstream>
#include <fstream>
#include <array>

namespace Util {
    template<typename AType, typename BType, typename FType, typename RType = AType>
    constexpr const RType lerp(const AType &a, const BType &b, const FType &factor) {
        return a * (FType(1) - factor) + b * factor;
    }
}

struct CSV {
    struct CSVRow {
        std::string row;
        std::istringstream ss;

        CSVRow(const std::string &row):
            row(row),
            ss(row)
        {}

        CSVRow():CSVRow(std::string()){}

        bool next(std::string &out) {
            return bool(std::getline(ss, out, ','));
        }

        bool skip(const int &count) {
            std::string v;
            for (int i = 0; i < count; i++)
                if (!next(v))
                    return false;

            return true;
        }
        
        template<typename T>
        bool parse_next(T &out) {
            std::string n;
            if (!next(n))
                return false;

            std::istringstream ss(n);
            ss >> out;

            return true;
        }
    };

    CSV(){}
    CSV(const char *path) {
        open(path);
    }

    std::ifstream s;

    bool next(CSVRow &row) {
        std::string line;

        if (!std::getline(s, line))
            return false;

        row = CSVRow(line);
        return true;
    }

    bool skip(const int &count) {
        std::string line;

        for (int i = 0; i < count; i++)
            if (!std::getline(s, line))
                return false;
            
        return true;
    }

    bool open(const char *path) {
        std::ifstream f(path);

        bool open = f.is_open();

        s = std::move(f);

        return open;
    }
};

struct ColId {
    int group_id, field_id;

    ColId():ColId(0,0){}

    ColId(const int &group_id, const int &field_id):
        group_id(group_id),
        field_id(field_id)
    {}

    bool operator==(const ColId &other) const {
        return other.group_id == group_id && other.field_id == field_id;
    }

    bool operator!=(const ColId &other) const {
        return !(this->operator==(other));
    }

    std::string to_string() const {
        return std::format("{}:{}", group_id, field_id);
    }

    friend std::ostream &operator<<(std::ostream &s, const ColId &col_id) {
        s << col_id.to_string();
        return s;
    }
};  

template<typename TIME_TYPE = float, typename VALUE_TYPE = float>
struct PointT {
    using value_type = VALUE_TYPE;
    using time_type = TIME_TYPE;

    time_type time;
    value_type value;

    PointT():PointT(0,0){}

    PointT(const time_type &time, const value_type &value):
        time(time),
        value(value)
    {}

    float get_factor(const PointT &other, const float &time) {
        return (time - this->time) / (other.time - this->time);
    }

    PointT interpolate(const PointT &other, const float &factor) {
        PointT ret;

        ret.time = Util::lerp(this->time, other.time, factor);
        ret.value = Util::lerp(this->value, other.value, factor);

        return ret;
    }

    friend std::ostream &operator<<(std::ostream &s, const PointT &p) {
        s << p.time << " " << p.value << ", ";
        return s;
    }
};

using Point = PointT<>;

template<typename P = Point>
std::ostream &operator<<(std::ostream &s, const std::vector<P> &v) {
    for (const auto &p : v)
        s << p;
    return s;
}

template<typename Point = ::Point>
struct ColumnT : public ColId {
    using ColId::operator!=;
    using ColId::operator==;
    using point_type = Point;
    using value_type = point_type::value_type;
    using time_type = point_type::time_type;

    static ColumnT INVALID;

    std::string name;
    std::vector<point_type> rows;

    ColumnT():ColumnT(std::string(), ColId()){}

    ColumnT(const std::string &name, const ColId &col_id):
        ColId(col_id),
        name(name)
    {}

    int binary_index(const time_type &time, const int &start, const int &end, const int &depth=0) const {
        const int range = end - start;

        if (range < 2)
            return start;
        
        const int mid = start + (range / 2);
        const auto &middle = rows[mid];

        if (middle.time <= time)
            return binary_index(time, mid, end, depth + 1);
        else
            return binary_index(time, start, mid, depth + 1);
    }

    int binary_index(const time_type &time) const {
        return binary_index(time, 0, get_row_count());
    }

    const int get_row_count() const {
        return rows.size();
    }

    const bool indexed_pair(const int &index, int &v1, int &v2) const {
        if (index < 1) {
            if (get_row_count() < 1)
                return false;
            v1 = 0;
            v2 = 1;
            return true;   
        }

        if (index >= get_row_count() - 1) {
            const int size = get_row_count();
            v1 = size - 2;
            v2 = size - 1;
            return true;
        }

        v1 = index;
        v2 = index + 1;
        return true;
    }

    const bool indexed_pair(const int &index, point_type &v1, point_type &v2) const {
        int a, b;

        if (!indexed_pair(index, a, b))
            return false;

        v1 = rows[a];
        v2 = rows[b];

        return true;
    }

    const bool time_pair(const time_type &time, int &v1, int &v2) const {
        const int i = binary_index(time);
        return indexed_pair(i, v1, v2);
    }

    const bool time_pair(const time_type &time, point_type &v1, point_type &v2) const {
        int a, b;

        if (!time_pair(time, a, b))
            return false;
        
        v1 = rows[a];
        v2 = rows[b];
        return true;
    }

    point_type interpolate_row(const time_type &time) {
        point_type v1, v2;

        if (!time_pair(time, v1, v2))
            return point_type();

        const float factor = v1.get_factor(v2, time);

        return v1.interpolate(v2, factor);
    }

    value_type differentiate_interpolated(const time_type &time, const time_type &time_width = 0.1) {
        const auto v1 = interpolate_row(time - time_width);
        const auto v2 = interpolate_row(time + time_width);
        
        return (v2.value - v1.value) / (v2.time - v1.time);
    }

    value_type differentiate(const int &index, const int &index_width = 1) {
        point_type v1, v2;

        
    }

    std::string to_string() const {
        return std::format("{} {}", ColId::to_string(), name);
    }

    friend std::ostream &operator<<(std::ostream &s, const ColumnT &column) {
        s << column.to_string();
        return s;
    }
};

template<> ColumnT<> ColumnT<>::INVALID("Invalid", {0, 0});

using Column = ColumnT<>;

template<typename Column = ::Column>
struct LogT {
    std::vector<Column> columns;
    using value_type = Column::value_type;
    using point_type = Column::point_type;
    using time_type = Column::time_type;

    void add_column(const Column &column) {
        column.rows.resize(get_height());
        columns.push_back(column);
    }

    void add_row(const value_type cols[]) {
        const int w = get_width();

        for (int i = 0; i < w; i++)
            columns[i].rows.push_back(cols[i]);
    }

    std::vector<point_type> get_row(const int &row) {
        std::vector<point_type> ret;
        const auto w = get_width();
        ret.reserve(w);

        for (int col = 0; col < w; col++)
            ret.push_back(get_value(col, row));

        return ret;
    }

    std::vector<point_type> interpolate_row(const time_type &time) {
        std::vector<point_type> ret;
        const auto w = get_width();
        ret.reserve(w);

        for (int col = 0; col < w; col++)
            ret.push_back(columns[col].interpolate_row(time));
        
        return ret;
    }

    size_t get_width() const {
        return columns.size();
    }

    point_type get_value(const int &column, const int &row) {
        return columns[column].rows[row];
    }

    point_type get_value(const ColId &col_id, const int &row) {
        return get_column(col_id)->row;
    }

    void get_time_minmax(time_type &min, time_type &max) {
        if (get_height() < 1)
            return;
        
        min = columns[0].rows.front().time;
        max = columns[0].rows.back().time;
    }

    size_t get_height() const {
        if (get_width() > 0)
            return columns[0].rows.size();
        return 0;
    }

    Column *get_column(const ColId &col_id) {
        auto it = std::find(columns.begin(), columns.end(), col_id);
        if (it != columns.end())
            return &*it;
        return nullptr;
    }

    Column *get_column(const int &group_id, const int &field_id) {
        return get_column({group_id, field_id});
    }

    Column &get_columnr(const ColId &col_id) {
        Column *col = get_column(col_id);
        if (col)
            return *col;
        return Column::INVALID;
    }

    void parse(CSV &csv) {
        CSV::CSVRow row;
        csv.skip(2);
        
        csv.next(row);

        row.skip(2);

        while (true) {
            Column col;
            std::string field;

            if (!row.next(field))
                break;
            
            col.group_id = strtol(field.c_str()+1, nullptr, 10);

            if (!row.next(field))
                break;

            col.field_id = strtol(field.c_str()+1, nullptr, 10);

            if (col.group_id == 0 && col.field_id == 0)
                continue;

            columns.push_back(col);
        }

        csv.skip(2);

        csv.next(row);

        row.skip(2);

        for (int i = 0; i < columns.size(); i++) {
            auto &col = columns[i];

            row.next(col.name);

            row.skip(1);
        }

        csv.skip(1);

        while (csv.next(row)) {
            row.skip(1);

            for (int i = 0; i < columns.size(); i++) {
                auto &col = columns[i];

                typename::Column::point_type p;

                row.parse_next(p.time);
                row.parse_next(p.value);

                col.rows.push_back(p);
            }
        }
    }
};

using Log = LogT<>;

int main(int argc, char **argv) {
    Log log;

    /*
    log.add_column({"Engine Speed", {2, 0}});
    log.add_column({"Injection Timing", {2, 2}});
    log.add_column({"Mass Air Flow", {2, 3}});
    log.add_column({"Throttle Valve Angle", {3, 2}});
    */

    CSV csv(argv[1]);
    log.parse(csv);

    /*
    for (int row = 0; row < log.get_height(); row++) {
        auto v = log.get_row(row);
        std::cout << row << ": ";
        std::cout << v;
        std::cout << std::endl;
    }
    */

    Log::time_type min, max;
    log.get_time_minmax(min, max);

    for (Log::time_type s = min; s < max; s += 1.0) {
        auto v = log.interpolate_row(s);
        std::cout << v << std::endl;
    }

    for (int col = 0; col < log.get_width(); col++) {
        std::cout << log.columns[col] << std::endl;
    }

    std::cout << std::format("Columns: {}, Rows: {}\n", log.get_width(), log.get_height());

    return 0;
}