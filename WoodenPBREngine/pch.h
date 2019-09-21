// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

#ifndef PCH_H
#define PCH_H

#define WPBR_USING using namespace WoodenPBR;
#define WPBR_BEGIN namespace WoodenPBR {
#define WPBR_END }

#include <cstddef>
#include "stdint.h"
#include <string>
#include <assert.h>
#include <algorithm>
#include "WoodenECS/WECS.h"
#include "WoodenECS/Job.h"
#include "WoodenMathLibrarry/DVector.h"

//using uint32_t = typename std::uint32_t;

#define PI 3.14

using namespace wml;
using namespace wal;
using namespace wecs;

using efloat = typename float;

// TODO: add headers that you want to pre-compile here

#endif //PCH_H
