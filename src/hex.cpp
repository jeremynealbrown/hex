#include "hex.h"

using namespace hex;

// ------------------------------------------------------------
//
// cell
//
// ------------------------------------------------------------

std::vector<point> cell::flat_vertices = [] {
    float theta = 0.0f;
    float inc = M_PI / 3.0f;
    std::vector<point> vertices;
    for(size_t i = 0; i < 6; i++) {
        vertices.push_back({std::cos(theta), std::sin(theta)});
        theta += inc;
    } 
    return vertices;
}();

std::vector<point> cell::sharp_vertices = [] {
    float inc = M_PI / 3.0f;
    float theta = inc * 0.5;
    std::vector<point> vertices;
    for(size_t i = 0; i < 6; i++) {
        vertices.push_back({std::cos(theta), std::sin(theta)});
        theta += inc;
    } 
    return vertices;
}();

cell cell::round(float x, float y, float z) {
    int xr = std::round(x);
    int yr = std::round(y);
    int zr = std::round(z);
    int xd = std::abs(x - xr);
    int yd = std::abs(y - yr);
    int zd = std::abs(z - zr);

    if(xd > yd && xd > zd) {
	xr = -yr - zr;
    } else if(yd > zd) {
	yr = -xr - zr;
    } else {
	zr = -xr - yr;
    }

    return cell(xr, yr, zr);
}

cell::cell() : x(0), y(0), z(0) {}

cell::cell(int32_t _x, int32_t _y, int32_t _z)
    : x(_x), y(_y), z(_z) {
    if(x + y + z != 0) throw(std::runtime_error("Invalid cell"));
}

cell cell::operator+(const cell& rhs) const {
    return cell(x + rhs.get_x(), y + rhs.get_y(), z + rhs.get_z());
}

void cell::operator+=(const cell& rhs) {
    x += rhs.get_x();
    y += rhs.get_y();
    z += rhs.get_z();
}

void cell::operator-=(const cell& rhs) {
    x -= rhs.get_x();
    y -= rhs.get_y();
    z -= rhs.get_z();
}

cell cell::operator-(const cell& rhs) const {
    return cell(x - rhs.get_x(), y - rhs.get_y(), z - rhs.get_z());
}

cell cell::operator*(float scalar) const {
    return cell(std::round(x * scalar), std::round(y * scalar), std::round(z * scalar));
}

bool cell::operator==(const cell& rhs) const {
    return x == rhs.get_x() && y == rhs.get_y() && z == rhs.get_z();
}

uint32_t cell::distance(const cell& other) {
    return (std::abs(x - other.get_x()) + std::abs(y - other.get_y()) + std::abs(z - other.get_z())) / 2;
}

std::ostream& operator<<(std::ostream& os, cell const& c) {
   os << "cell(" << std::to_string(c.get_x()) << ", " + std::to_string(c.get_y()) << ", " + std::to_string(c.get_z()) << ")";
   return os;
}

// ------------------------------------------------------------
//
// lattice
//
// ------------------------------------------------------------

point lattice::cell_to_point(cell& c, orientation o, float r) {
    float x = 0;
    float y = 0;
    if(o == orientation::flat) {
        x = r * (3.0/2.0) * c.get_x();
        y = r * sqrt(3.0) * (c.get_y() + c.get_x() * 0.5);
    } else {
        x = r * sqrt(3.0) * (c.get_x() + c.get_y() * 0.5);
        y = r * 3.0/2.0 * c.get_y();
    }

    return {x, y};
}

cell lattice::point_to_cell(point& p, orientation o, float r) {
    float x = 0;
    float y = 0;
    if(o == orientation::flat) {
   	x = p.x * 2/3 / r;
        y = (-p.x / 3 + sqrt(3)/3 * p.y) / r; 
    } else {
	x = (p.x * sqrt(3)/3 - p.y / 3) / r;
        y = p.y * 2/3 / r;
    }

    return cell::round(x, y, -x-y);
}

std::unordered_set<cell> lattice::get_neighbors(cell& c) {
    std::unordered_set<cell> result;
    for(size_t i = 0; i < neighbors().size(); i++) {
        result.emplace(c + neighbors()[i]);
    }

    return result;
}

cell lattice::get_neighbor(cell& c, uint8_t side) {
    if(side > 5) throw std::runtime_error("Attempt to get neighbor " + std::to_string(side) + " out of range");
    return c + lattice::neighbors().at(side);
}

void lattice::operator+=(lattice& rhs) { 
    insert(rhs.begin(), rhs.end());
}

void lattice::operator+=(const lattice& rhs) { 
    insert(rhs.begin(), rhs.end());
}

void lattice::operator-=(lattice& rhs) {
    for(auto & c : rhs) {
        if(count(c) != 0) erase(c);
    } 
}

void lattice::operator-=(const lattice& rhs){
    for(auto & c : rhs) {
        if(count(c) != 0) erase(c);
    }
}

// ------------------------------------------------------------
//
// layout
//
// ------------------------------------------------------------

lattice layout::hexagonal(int radius) {
    lattice result;
    for (int x = -radius; x <= radius; x++) {
	int y1 = std::max(-radius, -x - radius);
	int y2 = std::min(radius, -x + radius);
	for (int y = y1; y <= y2; y++) {
	    result.emplace(cell(x, y, -x-y));
	}
    }

    return result;
}

lattice layout::rectangular(int width, int height) {
    lattice result;
    for (int y = 0; y < height; y++) {
        int32_t y_offset = y >> 1;
        for (int x = -y_offset; x < width - y_offset; x++) {
            result.emplace(cell(x, y, -x-y));
        }
    }
    return result;  
}

lattice layout::parallelogram(int width, int height, options direction) {
    lattice result;
    int w = width * 0.5;
    int h = height * 0.5;

    for (int x = -w; x <= w; x++) {
        for (int y = -h; y <= h; y++) {
            if(direction == options::standard)
                result.emplace(cell(x, y, -x-y));
            if(direction == options::flipped)
                result.emplace(cell(-x-y, x, y));
            if(direction == options::vertical)
                result.emplace(cell(x, -x-y, y));
        }
    }
    return result;
}


lattice layout::triangular(int base, options direction) {
    lattice result;
    int h = std::floor(base * 0.5);
    int hh = std::floor(h * 0.5);
    if((hh % 2) != 0) hh--;
    for (int x = 0; x <= base; x++) {
	for (int y = 0; y <= base - x; y++) {
	    if(direction == options::standard)
		result.emplace(cell(x, -x-y, y));
	    if(direction == options::flipped)
		result.emplace(cell(x, y, -x-y));
	}
    }
    return result;  
}
