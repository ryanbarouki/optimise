// Optimize the best place to put 2 delivery depots
// Uses hill finding methods as the optimiser with a global search on top
// It relies on GBPlaces.csv being in the same directory as the main.cpp file

#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
#include <sstream>
#define PI 3.14159
#include <vector>
#include <string>

using namespace std;

// create a structure for the place info to be entered
struct gbplaces{
    string place, type;
    double pop, lat, longitude;
};

// random number generator
double random_number ( double upper, double lower, int n ) {
    double r;
    r = lower + ( rand() % (n+1) * (1./n) * (upper-lower));
    return r;
}

// function to calculate the greate circle distance
double haversine (double lat1, double lat2, double long1, double long2){
    // convert to radians
    lat1 = lat1*(PI/180);
    lat2 = lat2*(PI/180);
    long1 = long1*(PI/180);
    long2 = long2*(PI/180);

    double R = 3958.75;
    double dLat = lat2 - lat1;
    double dLong = long2 - long1;
    double a = pow(sin(dLat/2),2) + cos(lat1)*cos(lat2)*sin(dLong/2)*sin(dLong/2);
    double c = 2*atan2(sqrt(a), sqrt(1-a));
    double distance = R * c;

    return distance;
}

// function to calculate the total fitness from a point to all the places in gbplaces
// the 'fitness' in this case is the distance from a point to the place / sqrt(the population of that place)
double totalDistance (vector <gbplaces> place_info, double lat_try1, double long_try1, double lat_try2, double long_try2){
    double total_dist, to_hub1, to_hub2;
    // loop through all places
    for (int i = 0; i < place_info.size(); i++){
        // calculate the distance from a place to both hubs
        to_hub1 = haversine(lat_try1, place_info[i].lat, long_try1, place_info[i].longitude)/sqrt(place_info[i].pop);
        to_hub2 = haversine(lat_try2, place_info[i].lat, long_try2, place_info[i].longitude)/sqrt(place_info[i].pop);
        if (to_hub1 < to_hub2){
            // if the distance to hub 1 is less than to hub 2 then add it to the total
            total_dist += to_hub1;
        } else {
            // if the distance to hub 2 is less then add that distance to the total
            total_dist += to_hub2;
        }

    }
    return total_dist;
}

int main() {
    // Declare variables
    double lat_try1, long_try1, lat_try2, long_try2, value, oldValue, newValue, dlat1, dlong1, dlat2, dlong2;
    double step = 0.01;
    double globalMin = 1e10; // big number to start with to be reduced later
    double gl_lat1, gl_long1, gl_lat2, gl_long2;
    string line;
    vector<gbplaces> place_info; // create a vector of structures of type gbplaces
    gbplaces temp; // temporary variable to push back onto array

    srand(time(NULL)); // seed the random generator

    // read in the file
    ifstream gbplaces("../GBplaces.csv");

    // declare variables to extract the appropriate data from gbplaces
    string place, type, population, latitude, longitude;

    getline(gbplaces, line); // skips the first line

    if (gbplaces.is_open()) {
        // yes it is
        while (getline(gbplaces, place, ',')) {
            // separates the file by commas then the last one by the new line
            getline(gbplaces, type, ',');
            getline(gbplaces, population, ',');
            getline(gbplaces, latitude, ',');
            getline(gbplaces, longitude);

            // add the data to the temporary vector
            temp.place = place;
            temp.type = type;
            temp.pop = atof(population.c_str());
            temp.lat = atof(latitude.c_str());
            temp.longitude = atof(longitude.c_str());
            // add the data to the vector of structures
            place_info.push_back(temp);

        }
        gbplaces.close(); // close the file
    } else {
        // file did not open
        cout << "Unable to open the file for reading\n";
        // stop the program
        exit(1);
    }

    // MAIN BODY OF CODE
    for (int k = 0; k < 50; k++) {

        // first pick a random starting point for latitude and longitude of the 2 hubs
        lat_try1 = random_number(50.3, 57.2, 100);
        long_try1 = random_number(-4.3, -1.3, 100);

        lat_try2 = random_number(50.3, 57.2, 100);
        long_try2 = random_number(-4.3, -1.3, 100);

        // now work out the value of the function at point x,y
        value = totalDistance(place_info, lat_try1, long_try1, lat_try2, long_try2);

        // cout a few values to see if it is working
        cout << lat_try1 << ", " << long_try1 << " || " << lat_try2 << ", " << long_try2 << " ---> " << value << "\n";
        do {
            // now look around the current point and see if we can go somewhere where the value is lower
            oldValue = value;

            // 4 loops are required to move the lat and long coordinates of the 2 hubs independently
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    for (int k = -1; k <= 1; k++) {
                        for (int n = -1; n <= 1; n++) {
                            if (i == 0 && j == 0 && k == 0 && n == 0) {
                            } else {
                                // calculate the value at this new point
                                newValue = totalDistance(place_info, lat_try1 + step * i, long_try1 + step * j,
                                                         lat_try2 + step * k, long_try2 + step * n);

                                // if this value is less than the previous value, save the value and it's distance from the last
                                if (newValue <= value) {
                                    dlat1 = i;
                                    dlong1 = j;
                                    dlat2 = k;
                                    dlong2 = n;
                                    value = newValue;
                                }
                            }
                        }
                    }
                }
            }

            // update to new position and new value
            lat_try1 += step * dlat1;
            long_try1 += step * dlong1;
            lat_try2 += step * dlat2;
            long_try2 += step * dlong2;

        } while (value < oldValue); // continue this loop while the current value is less than old value

        // compare the 'local minimum' obtained from the do-while loop above to the global minimum.
        if (value < globalMin) {
            globalMin = value; // if it is less make it the new global minimum
            // set the global minimum coordinates
            gl_lat1 = lat_try1;
            gl_long1 = long_try1;
            gl_lat2 = lat_try2;
            gl_long2 = long_try2;
        }

    }

    // This bit just finds the nearest place in GBPlaces to give the user an idea where the hubs are
    // declare some more variables
    double nearest1, nearest2;
    double nearest1_temp = 1e10;
    double nearest2_temp = 1e10;
    vector<string> hub1_places;
    vector<string> hub2_places;
    string nearest_place_1, nearest_place_2;

    for (int i; i < place_info.size(); i++) {
        // find the distance to the 2 hubs
        nearest1 = haversine(gl_lat1, place_info[i].lat, gl_long1, place_info[i].longitude);
        nearest2 = haversine(gl_lat2, place_info[i].lat, gl_long2, place_info[i].longitude);

        // search for the smallest distance of a place to hub 1
        if (nearest1 < nearest1_temp) {
            nearest_place_1 = place_info[i].place;
            nearest1_temp = nearest1;
        }
        // search for the smallest distance of a place to hub 2
        if (nearest2 < nearest2_temp) {
            nearest_place_2 = place_info[i].place;
            nearest2_temp = nearest2;
        }
    }

    // Output the result to the user in the console
    cout << "BEST PLACE FOR 2 HUBS: \n";
    cout << "Hub 1: " << gl_lat1 << ", " << gl_long1 << " || Hub 2: " << gl_lat2 << ", " << gl_long2
         << " || Global Min: " << globalMin << " \n";
    cout << "This is close to " << nearest_place_1 << " and " << nearest_place_2 << " respectively. \n";

}