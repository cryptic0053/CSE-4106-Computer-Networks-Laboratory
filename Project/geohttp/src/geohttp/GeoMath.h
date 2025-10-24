#pragma once
#include <cmath>

inline double deg2rad(double d) { return d; } // INET stores as radians already in our params

inline double haversine_km(double lat1, double lon1, double lat2, double lon2) {
    // all radians
    static const double R = 6371.0; // km
    double dlat = lat2 - lat1, dlon = lon2 - lon1;
    double a = std::sin(dlat/2)*std::sin(dlat/2) + std::cos(lat1)*std::cos(lat2)*std::sin(dlon/2)*std::sin(dlon/2);
    double c = 2*std::atan2(std::sqrt(a), std::sqrt(1-a));
    return R * c;
}
