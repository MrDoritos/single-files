#include <iostream>
#include <cmath>

using namespace std;

class Point {
	private:
		double x, y;

	public:
		Point(double x, double y);
		void setX(double x);
		void setY(double y);
		double dist(const Point &p);
};

Point::Point(double x, double y) {
	this->x = x;
	this->y = y;
}

void Point::setX(double x) {
	this->x = x;
}

void Point::setY(double y) {
	this->y = y;
}

double Point::dist(const Point &p) {
	return sqrt(pow(p.x-x,2)+pow(p.y-y,2));
}

int main() {
	Point a{3,5}, b{9,9};
	cout<<"Distance: "<<a.dist(b)<<endl;
	return 0;
}
