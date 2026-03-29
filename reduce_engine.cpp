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

    void add_column(const Column &column) {
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

    size_t get_width() const {
        return columns.size();
    }

    point_type get_value(const int &column, const int &row) {
        return columns[column].rows[row];
    }

    point_type get_value(const ColId &col_id, const int &row) {
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

    for (int row = 0; row < log.get_height(); row++) {
        auto v = log.get_row(row);
        std::cout << row << ": ";
        std::cout << v;
        std::cout << std::endl;
    }

    for (int col = 0; col < log.get_width(); col++) {
        std::cout << log.columns[col] << std::endl;
    }

    std::cout << std::format("Columns: {}, Rows: {}\n", log.get_width(), log.get_height());

    return 0;
}