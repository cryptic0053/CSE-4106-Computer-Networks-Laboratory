#pragma once
#include <cmath>

inline double deg2rad(double deg) {
    return deg * M_PI / 180.0;
}

inline double haversine_km(double lat1, double lon1, double lat2, double lon2) {
    // Input: degrees; convert to radians
    static const double R = 6371.0; // km
    double lat1r = deg2rad(lat1);
    double lon1r = deg2rad(lon1);
    double lat2r = deg2rad(lat2);
    double lon2r = deg2rad(lon2);

    double dlat = lat2r - lat1r;
    double dlon = lon2r - lon1r;

    double a = std::sin(dlat/2)*std::sin(dlat/2) +
               std::cos(lat1r)*std::cos(lat2r)*std::sin(dlon/2)*std::sin(dlon/2);
    double c = 2*std::atan2(std::sqrt(a), std::sqrt(1-a));
    return R * c;
}
