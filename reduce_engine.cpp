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

namespace Util {
    template<typename AType, typename BType, typename FType, typename RType = AType>
    constexpr const RType lerp(const AType &a, const BType &b, const FType &factor) {
        return a * (FType(1) - factor) + b * factor;
    }
}

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

    PointT(const time_type &time, const value_type &value):
        time(time),
        value(value)
    {}
};

using Point = PointT<>;

template<typename Point = ::Point>
struct ColumnT : public ColId {
    using ColId::operator!=;
    using ColId::operator==;
    using point_type = Point;
    using value_type = point_type::value_type;
    using time_type = point_type::time_type;

    static ColumnT INVALID;

    std::string name;

    ColumnT():ColumnT(std::string(), ColId()){}

    ColumnT(const std::string &name, const ColId &col_id):
        ColId(col_id),
        name(name)
    {}

    std::vector<point_type> rows;

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

    void add_column(const Column &column) {
        columns.push_back(column);
    }

    void add_row(const value_type cols[]) {
        const int w = get_width();

        for (int i = 0; i < w; i++)
            columns[i].rows.push_back(cols[i]);
    }

    size_t get_width() const {
        return columns.size();
    }

    value_type get_value(const int &column, const int &row) {
        return columns[column].rows[row];
    }

    value_type get_value(const ColId &col_id, const int &row) {
        return get_column(col_id)->row;
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
};

using Log = LogT<>;

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

    bool open(const char *path) {
        std::ifstream f(path);

        bool open = f.is_open();

        s = std::move(f);

        return open;
    }
};

int main(int argc, char **argv) {
    Log log;

    log.add_column({"Engine Speed", {2, 0}});
    log.add_column({"Injection Timing", {2, 2}});
    log.add_column({"Mass Air Flow", {2, 3}});
    log.add_column({"Throttle Valve Angle", {3, 2}});

    std::cout << log.get_columnr({2, 0}) << std::endl;
    std::cout << log.get_columnr({3, 2}) << std::endl;

    CSV csv(argv[1]);

    CSV::CSVRow row;
    while (csv.next(row)) {
        float n;
        while (row.parse_next(n)) {
            std::cout << n << ", ";
        }
        std::cout << "\n";
    }
}