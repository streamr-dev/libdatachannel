//
//  helpers.h
//  libdatachannel
//
//  Created by Savolainen, Petri T E on 02/03/2021.
//

#ifndef helpers_h
#define helpers_h

#include <random>
#include <string>

using namespace std;

namespace Helpers
{
// Helper function to generate a random String
inline string randomString(size_t length)
    {
    static const string characters(
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    string id(length, '0');
    default_random_engine rng(random_device{}());
    uniform_int_distribution<int> dist(0, int(characters.size() - 1));
    generate(id.begin(), id.end(), [&]() { return characters.at(dist(rng)); });
    return id;
    }

inline int formatRate(int bytes)
    {
    return bytes / 1024;
    }

}

#endif /* helpers_h */
