#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <iterator>
#include <string.h>
#include <algorithm>
#include <map>
#include <vector>
#include <format>

/*              Quantity
                1-25    26-50   51-75 76-100
    Print   1
            2
            3
            4
            5

*/

typedef float price;
typedef int size;
typedef int quantity;

std::vector<std::string_view> shirt_sizes = {
    "Y", "S", "M", "L", "XL", "XL2", "XL3", "XL4", "XL5"
};

std::vector<quantity> base_quantites = {
    24, 49, 199, 299
};

std::vector<price> base_prices = {
    19.69, 9.69, 7.70, 6.70
};

std::vector<size> base_sizes = {
    0, 1, 5
};

std::vector<price> base_size_price_increase = {
    0, 1.3, 1.95
};

std::vector<std::map<quantity, std::map<size, price>>> order_types;

void gen_orders() {
    order_types.resize(5);

    for (int i = 0; i < 5; i++) {
        auto &order = order_types[i];
        for (int j = 0; j < base_quantites.size(); j++) {
            const auto Quantity = base_quantites[j];
            auto &price_map = order_types[i][j] = std::map<size, price>();
            const auto basePrice = base_prices[j];
            //for (int k = 0; k < base_prices.size(); k++) {
                for (int l = 0; l < base_sizes.size(); l++) {
                    const auto FinalPrice = basePrice + base_size_price_increase[l] + ((i * 1)) + int(j / 3);
                    const auto Size = base_sizes[l];
                    price_map[Size] = FinalPrice;
                }
            //}
        }
    }
}

void print_orders() {
    for (int order = 0; order < order_types.size(); order++) {
        const auto &print = order_types[order];
        std::cout << std::format("{:>3}\t", order+1);
        quantity last = 12;
        for (const auto &[Quantity, prices] : print) {
            const auto rquant = base_quantites[Quantity];
            std::cout << std::format("{:>3} - {:>3}\t", last, rquant);
            last = rquant;
        }
        std::cout << std::endl;
        for (int i = 0; i < base_sizes.size(); i++) {
            std::cout << "\t";
            const auto &size_ind = base_sizes[i];
            const auto &size_str = shirt_sizes[size_ind];
            for (const auto &[Quantity, prices] : print) {
                const auto &pric = prices.at(size_ind);
                std::cout << std::format("{:>3} ${:<4.2f}\t", size_str, pric);
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

int main() {
    int s;
    char buf[50];
    std::cout << "enter shirt size: ";
    scanf("%i%s", &s, buf);
    std::cout << s << ":" << buf << std::endl;
    gen_orders();
    print_orders();
    return 0;
}

