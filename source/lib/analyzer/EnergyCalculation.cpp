#include "EnergyCalculation.h"

#include "DCTTransforms.h"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace {

// TODO: Convert this into a integer operation. That should be possible in 16 bit
//       arithmetic with the same precision.

static const double weights_dct8[64] = {
    0.000000, 0.000000, 0.368689, 0.369319, 0.370132, 0.371127, 0.372307, 0.373673,
    0.000000, 0.369319, 0.371127, 0.373673, 0.376971, 0.381043, 0.385911, 0.391606,
    0.368689, 0.371127, 0.375227, 0.381043, 0.388653, 0.398161, 0.409698, 0.423427,
    0.369319, 0.373673, 0.381043, 0.391606, 0.405618, 0.423427, 0.445484, 0.472367,
    0.370132, 0.376971, 0.388653, 0.405618, 0.428522, 0.458281, 0.496125, 0.543691,
    0.371127, 0.381043, 0.398161, 0.423427, 0.458281, 0.504800, 0.565901, 0.645649,
    0.372307, 0.385911, 0.409698, 0.445484, 0.496125, 0.565901, 0.661121, 0.791065,
    0.373673, 0.391606, 0.423427, 0.472367, 0.543691, 0.645649, 0.791065, 1.000000,
};

static const double weights_dct16[256] = {
    0.000000, 0.000000, 0.367930, 0.367969, 0.368020, 0.368082, 0.368155, 0.368239, 0.368334,
    0.368441, 0.368559, 0.368689, 0.368829, 0.368981, 0.369145, 0.369319, 0.000000, 0.367969,
    0.368082, 0.368239, 0.368441, 0.368689, 0.368981, 0.369319, 0.369703, 0.370132, 0.370606,
    0.371127, 0.371694, 0.372307, 0.372966, 0.373673, 0.367930, 0.368082, 0.368334, 0.368689,
    0.369145, 0.369703, 0.370363, 0.371127, 0.371994, 0.372966, 0.374043, 0.375227, 0.376517,
    0.377916, 0.379424, 0.381043, 0.367969, 0.368239, 0.368689, 0.369319, 0.370132, 0.371127,
    0.372307, 0.373673, 0.375227, 0.376971, 0.378909, 0.381043, 0.383376, 0.385911, 0.388653,
    0.391606, 0.368020, 0.368441, 0.369145, 0.370132, 0.371405, 0.372966, 0.374821, 0.376971,
    0.379424, 0.382184, 0.385258, 0.388653, 0.392377, 0.396439, 0.400849, 0.405618, 0.368082,
    0.368689, 0.369703, 0.371127, 0.372966, 0.375227, 0.377916, 0.381043, 0.384618, 0.388653,
    0.393162, 0.398161, 0.403667, 0.409698, 0.416277, 0.423427, 0.368155, 0.368981, 0.370363,
    0.372307, 0.374821, 0.377916, 0.381607, 0.385911, 0.390847, 0.396439, 0.402713, 0.409698,
    0.417429, 0.425941, 0.435277, 0.445484, 0.368239, 0.369319, 0.371127, 0.373673, 0.376971,
    0.381043, 0.385911, 0.391606, 0.398161, 0.405618, 0.414022, 0.423427, 0.433891, 0.445484,
    0.458281, 0.472367, 0.368334, 0.369703, 0.371994, 0.375227, 0.379424, 0.384618, 0.390847,
    0.398161, 0.406616, 0.416277, 0.427223, 0.439542, 0.453336, 0.468719, 0.485824, 0.504800,
    0.368441, 0.370132, 0.372966, 0.376971, 0.382184, 0.388653, 0.396439, 0.405618, 0.416277,
    0.428522, 0.442476, 0.458281, 0.476100, 0.496125, 0.518572, 0.543691, 0.368559, 0.370606,
    0.374043, 0.378909, 0.385258, 0.393162, 0.402713, 0.414022, 0.427223, 0.442476, 0.459969,
    0.479922, 0.502594, 0.528283, 0.557340, 0.590171, 0.368689, 0.371127, 0.375227, 0.381043,
    0.388653, 0.398161, 0.409698, 0.423427, 0.439542, 0.458281, 0.479922, 0.504800, 0.533305,
    0.565901, 0.603134, 0.645649, 0.368829, 0.371694, 0.376517, 0.383376, 0.392377, 0.403667,
    0.417429, 0.433891, 0.453336, 0.476100, 0.502594, 0.533305, 0.568819, 0.609834, 0.657188,
    0.711882, 0.368981, 0.372307, 0.377916, 0.385911, 0.396439, 0.409698, 0.425941, 0.445484,
    0.468719, 0.496125, 0.528283, 0.565901, 0.609834, 0.661121, 0.721021, 0.791065, 0.369145,
    0.372966, 0.379424, 0.388653, 0.400849, 0.416277, 0.435277, 0.458281, 0.485824, 0.518572,
    0.557340, 0.603134, 0.657188, 0.721021, 0.796503, 0.885951, 0.369319, 0.373673, 0.381043,
    0.391606, 0.405618, 0.423427, 0.445484, 0.472367, 0.504800, 0.543691, 0.590171, 0.645649,
    0.711882, 0.791065, 0.885951, 1.000000,
};

static const double weights_dct32[1024] = {
    0.000000, 0.000000, 0.367883, 0.367885, 0.367888, 0.367892, 0.367897, 0.367902, 0.367908,
    0.367915, 0.367922, 0.367930, 0.367939, 0.367948, 0.367958, 0.367969, 0.367981, 0.367993,
    0.368006, 0.368020, 0.368034, 0.368049, 0.368065, 0.368082, 0.368099, 0.368117, 0.368135,
    0.368155, 0.368175, 0.368195, 0.368217, 0.368239, 0.000000, 0.367885, 0.367892, 0.367902,
    0.367915, 0.367930, 0.367948, 0.367969, 0.367993, 0.368020, 0.368049, 0.368082, 0.368117,
    0.368155, 0.368195, 0.368239, 0.368285, 0.368334, 0.368386, 0.368441, 0.368499, 0.368559,
    0.368623, 0.368689, 0.368758, 0.368829, 0.368904, 0.368981, 0.369062, 0.369145, 0.369231,
    0.369319, 0.367883, 0.367892, 0.367908, 0.367930, 0.367958, 0.367993, 0.368034, 0.368082,
    0.368135, 0.368195, 0.368262, 0.368334, 0.368413, 0.368499, 0.368591, 0.368689, 0.368793,
    0.368904, 0.369021, 0.369145, 0.369275, 0.369411, 0.369554, 0.369703, 0.369858, 0.370020,
    0.370189, 0.370363, 0.370545, 0.370732, 0.370926, 0.371127, 0.367885, 0.367902, 0.367930,
    0.367969, 0.368020, 0.368082, 0.368155, 0.368239, 0.368334, 0.368441, 0.368559, 0.368689,
    0.368829, 0.368981, 0.369145, 0.369319, 0.369505, 0.369703, 0.369911, 0.370132, 0.370363,
    0.370606, 0.370861, 0.371127, 0.371405, 0.371694, 0.371994, 0.372307, 0.372631, 0.372966,
    0.373314, 0.373673, 0.367888, 0.367915, 0.367958, 0.368020, 0.368099, 0.368195, 0.368309,
    0.368441, 0.368591, 0.368758, 0.368942, 0.369145, 0.369365, 0.369603, 0.369858, 0.370132,
    0.370423, 0.370732, 0.371059, 0.371405, 0.371768, 0.372149, 0.372549, 0.372966, 0.373402,
    0.373857, 0.374329, 0.374821, 0.375330, 0.375859, 0.376406, 0.376971, 0.367892, 0.367930,
    0.367993, 0.368082, 0.368195, 0.368334, 0.368499, 0.368689, 0.368904, 0.369145, 0.369411,
    0.369703, 0.370020, 0.370363, 0.370732, 0.371127, 0.371548, 0.371994, 0.372467, 0.372966,
    0.373492, 0.374043, 0.374622, 0.375227, 0.375859, 0.376517, 0.377203, 0.377916, 0.378656,
    0.379424, 0.380219, 0.381043, 0.367897, 0.367948, 0.368034, 0.368155, 0.368309, 0.368499,
    0.368723, 0.368981, 0.369275, 0.369603, 0.369965, 0.370363, 0.370796, 0.371264, 0.371768,
    0.372307, 0.372881, 0.373492, 0.374138, 0.374821, 0.375539, 0.376295, 0.377087, 0.377916,
    0.378782, 0.379686, 0.380628, 0.381607, 0.382625, 0.383681, 0.384777, 0.385911, 0.367902,
    0.367969, 0.368082, 0.368239, 0.368441, 0.368689, 0.368981, 0.369319, 0.369703, 0.370132,
    0.370606, 0.371127, 0.371694, 0.372307, 0.372966, 0.373673, 0.374426, 0.375227, 0.376075,
    0.376971, 0.377916, 0.378909, 0.379951, 0.381043, 0.382184, 0.383376, 0.384618, 0.385911,
    0.387256, 0.388653, 0.390103, 0.391606, 0.367908, 0.367993, 0.368135, 0.368334, 0.368591,
    0.368904, 0.369275, 0.369703, 0.370189, 0.370732, 0.371334, 0.371994, 0.372714, 0.373492,
    0.374329, 0.375227, 0.376185, 0.377203, 0.378283, 0.379424, 0.380628, 0.381894, 0.383224,
    0.384618, 0.386076, 0.387600, 0.389190, 0.390847, 0.392572, 0.394365, 0.396228, 0.398161,
    0.367915, 0.368020, 0.368195, 0.368441, 0.368758, 0.369145, 0.369603, 0.370132, 0.370732,
    0.371405, 0.372149, 0.372966, 0.373857, 0.374821, 0.375859, 0.376971, 0.378160, 0.379424,
    0.380765, 0.382184, 0.383681, 0.385258, 0.386915, 0.388653, 0.390473, 0.392377, 0.394365,
    0.396439, 0.398600, 0.400849, 0.403188, 0.405618, 0.367922, 0.368049, 0.368262, 0.368559,
    0.368942, 0.369411, 0.369965, 0.370606, 0.371334, 0.372149, 0.373052, 0.374043, 0.375124,
    0.376295, 0.377556, 0.378909, 0.380355, 0.381894, 0.383528, 0.385258, 0.387085, 0.389010,
    0.391036, 0.393162, 0.395392, 0.397725, 0.400165, 0.402713, 0.405371, 0.408140, 0.411023,
    0.414022, 0.367930, 0.368082, 0.368334, 0.368689, 0.369145, 0.369703, 0.370363, 0.371127,
    0.371994, 0.372966, 0.374043, 0.375227, 0.376517, 0.377916, 0.379424, 0.381043, 0.382773,
    0.384618, 0.386577, 0.388653, 0.390847, 0.393162, 0.395600, 0.398161, 0.400849, 0.403667,
    0.406616, 0.409698, 0.412918, 0.416277, 0.419779, 0.423427, 0.367939, 0.368117, 0.368413,
    0.368829, 0.369365, 0.370020, 0.370796, 0.371694, 0.372714, 0.373857, 0.375124, 0.376517,
    0.378037, 0.379686, 0.381465, 0.383376, 0.385420, 0.387600, 0.389919, 0.392377, 0.394979,
    0.397725, 0.400621, 0.403667, 0.406867, 0.410225, 0.413745, 0.417429, 0.421281, 0.425306,
    0.429508, 0.433891, 0.367948, 0.368155, 0.368499, 0.368981, 0.369603, 0.370363, 0.371264,
    0.372307, 0.373492, 0.374821, 0.376295, 0.377916, 0.379686, 0.381607, 0.383681, 0.385911,
    0.388299, 0.390847, 0.393560, 0.396439, 0.399489, 0.402713, 0.406115, 0.409698, 0.413468,
    0.417429, 0.421584, 0.425941, 0.430503, 0.435277, 0.440269, 0.445484, 0.367958, 0.368195,
    0.368591, 0.369145, 0.369858, 0.370732, 0.371768, 0.372966, 0.374329, 0.375859, 0.377556,
    0.379424, 0.381465, 0.383681, 0.386076, 0.388653, 0.391415, 0.394365, 0.397509, 0.400849,
    0.404392, 0.408140, 0.412100, 0.416277, 0.420677, 0.425306, 0.430171, 0.435277, 0.440634,
    0.446248, 0.452127, 0.458281, 0.367969, 0.368239, 0.368689, 0.369319, 0.370132, 0.371127,
    0.372307, 0.373673, 0.375227, 0.376971, 0.378909, 0.381043, 0.383376, 0.385911, 0.388653,
    0.391606, 0.394773, 0.398161, 0.401774, 0.405618, 0.409698, 0.414022, 0.418596, 0.423427,
    0.428522, 0.433891, 0.439542, 0.445484, 0.451727, 0.458281, 0.465157, 0.472367, 0.367981,
    0.368285, 0.368793, 0.369505, 0.370423, 0.371548, 0.372881, 0.374426, 0.376185, 0.378160,
    0.380355, 0.382773, 0.385420, 0.388299, 0.391415, 0.394773, 0.398380, 0.402242, 0.406365,
    0.410756, 0.415424, 0.420377, 0.425623, 0.431172, 0.437035, 0.443221, 0.449743, 0.456612,
    0.463842, 0.471447, 0.479440, 0.487837, 0.367993, 0.368334, 0.368904, 0.369703, 0.370732,
    0.371994, 0.373492, 0.375227, 0.377203, 0.379424, 0.381894, 0.384618, 0.387600, 0.390847,
    0.394365, 0.398161, 0.402242, 0.406616, 0.411291, 0.416277, 0.421584, 0.427223, 0.433205,
    0.439542, 0.446248, 0.453336, 0.460821, 0.468719, 0.477048, 0.485824, 0.495068, 0.504800,
    0.368006, 0.368386, 0.369021, 0.369911, 0.371059, 0.372467, 0.374138, 0.376075, 0.378283,
    0.380765, 0.383528, 0.386577, 0.389919, 0.393560, 0.397509, 0.401774, 0.406365, 0.411291,
    0.416564, 0.422194, 0.428196, 0.434582, 0.441367, 0.448567, 0.456199, 0.464279, 0.472829,
    0.481867, 0.491416, 0.501500, 0.512144, 0.523373, 0.368020, 0.368441, 0.369145, 0.370132,
    0.371405, 0.372966, 0.374821, 0.376971, 0.379424, 0.382184, 0.385258, 0.388653, 0.392377,
    0.396439, 0.400849, 0.405618, 0.410756, 0.416277, 0.422194, 0.428522, 0.435277, 0.442476,
    0.450137, 0.458281, 0.466927, 0.476100, 0.485824, 0.496125, 0.507031, 0.518572, 0.530780,
    0.543691, 0.368034, 0.368499, 0.369275, 0.370363, 0.371768, 0.373492, 0.375539, 0.377916,
    0.380628, 0.383681, 0.387085, 0.390847, 0.394979, 0.399489, 0.404392, 0.409698, 0.415424,
    0.421584, 0.428196, 0.435277, 0.442848, 0.450930, 0.459545, 0.468719, 0.478479, 0.488853,
    0.499872, 0.511569, 0.523981, 0.537145, 0.551104, 0.565901, 0.368049, 0.368559, 0.369411,
    0.370606, 0.372149, 0.374043, 0.376295, 0.378909, 0.381894, 0.385258, 0.389010, 0.393162,
    0.397725, 0.402713, 0.408140, 0.414022, 0.420377, 0.427223, 0.434582, 0.442476, 0.450930,
    0.459969, 0.469623, 0.479922, 0.490901, 0.502594, 0.515041, 0.528283, 0.542367, 0.557340,
    0.573256, 0.590171, 0.368065, 0.368623, 0.369554, 0.370861, 0.372549, 0.374622, 0.377087,
    0.379951, 0.383224, 0.386915, 0.391036, 0.395600, 0.400621, 0.406115, 0.412100, 0.418596,
    0.425623, 0.433205, 0.441367, 0.450137, 0.459545, 0.469623, 0.480406, 0.491934, 0.504246,
    0.517388, 0.531409, 0.546360, 0.562299, 0.579288, 0.597392, 0.616684, 0.368082, 0.368689,
    0.369703, 0.371127, 0.372966, 0.375227, 0.377916, 0.381043, 0.384618, 0.388653, 0.393162,
    0.398161, 0.403667, 0.409698, 0.416277, 0.423427, 0.431172, 0.439542, 0.448567, 0.458281,
    0.468719, 0.479922, 0.491934, 0.504800, 0.518572, 0.533305, 0.549060, 0.565901, 0.583900,
    0.603134, 0.623687, 0.645649, 0.368099, 0.368758, 0.369858, 0.371405, 0.373402, 0.375859,
    0.378782, 0.382184, 0.386076, 0.390473, 0.395392, 0.400849, 0.406867, 0.413468, 0.420677,
    0.428522, 0.437035, 0.446248, 0.456199, 0.466927, 0.478479, 0.490901, 0.504246, 0.518572,
    0.533940, 0.550421, 0.568086, 0.587018, 0.607305, 0.629041, 0.652333, 0.677295, 0.368117,
    0.368829, 0.370020, 0.371694, 0.373857, 0.376517, 0.379686, 0.383376, 0.387600, 0.392377,
    0.397725, 0.403667, 0.410225, 0.417429, 0.425306, 0.433891, 0.443221, 0.453336, 0.464279,
    0.476100, 0.488853, 0.502594, 0.517388, 0.533305, 0.550421, 0.568819, 0.588590, 0.609834,
    0.632661, 0.657188, 0.683548, 0.711882, 0.368135, 0.368904, 0.370189, 0.371994, 0.374329,
    0.377203, 0.380628, 0.384618, 0.389190, 0.394365, 0.400165, 0.406616, 0.413745, 0.421584,
    0.430171, 0.439542, 0.449743, 0.460821, 0.472829, 0.485824, 0.499872, 0.515041, 0.531409,
    0.549060, 0.568086, 0.588590, 0.610682, 0.634486, 0.660134, 0.687774, 0.717570, 0.749697,
    0.368155, 0.368981, 0.370363, 0.372307, 0.374821, 0.377916, 0.381607, 0.385911, 0.390847,
    0.396439, 0.402713, 0.409698, 0.417429, 0.425941, 0.435277, 0.445484, 0.456612, 0.468719,
    0.481867, 0.496125, 0.511569, 0.528283, 0.546360, 0.565901, 0.587018, 0.609834, 0.634486,
    0.661121, 0.689906, 0.721021, 0.754667, 0.791065, 0.368175, 0.369062, 0.370545, 0.372631,
    0.375330, 0.378656, 0.382625, 0.387256, 0.392572, 0.398600, 0.405371, 0.412918, 0.421281,
    0.430503, 0.440634, 0.451727, 0.463842, 0.477048, 0.491416, 0.507031, 0.523981, 0.542367,
    0.562299, 0.583900, 0.607305, 0.632661, 0.660134, 0.689906, 0.722178, 0.757173, 0.795138,
    0.836348, 0.368195, 0.369145, 0.370732, 0.372966, 0.375859, 0.379424, 0.383681, 0.388653,
    0.394365, 0.400849, 0.408140, 0.416277, 0.425306, 0.435277, 0.446248, 0.458281, 0.471447,
    0.485824, 0.501500, 0.518572, 0.537145, 0.557340, 0.579288, 0.603134, 0.629041, 0.657188,
    0.687774, 0.721021, 0.757173, 0.796503, 0.839317, 0.885951, 0.368217, 0.369231, 0.370926,
    0.373314, 0.376406, 0.380219, 0.384777, 0.390103, 0.396228, 0.403188, 0.411023, 0.419779,
    0.429508, 0.440269, 0.452127, 0.465157, 0.479440, 0.495068, 0.512144, 0.530780, 0.551104,
    0.573256, 0.597392, 0.623687, 0.652333, 0.683548, 0.717570, 0.754667, 0.795138, 0.839317,
    0.887575, 0.940331, 0.368239, 0.369319, 0.371127, 0.373673, 0.376971, 0.381043, 0.385911,
    0.391606, 0.398161, 0.405618, 0.414022, 0.423427, 0.433891, 0.445484, 0.458281, 0.472367,
    0.487837, 0.504800, 0.523373, 0.543691, 0.565901, 0.590171, 0.616684, 0.645649, 0.677295,
    0.711882, 0.749697, 0.791065, 0.836348, 0.885951, 0.940331, 1.000000,
};

static const double E_norm_factor = 90;
static const double h_norm_factor = 18;

uint32_t calculateWeightedCoeffSum(unsigned blockSize, int16_t *coeffBuffer)
{
    uint32_t weightedSum = 0;

    auto weightFactorMatrix = weights_dct32;
    switch (blockSize)
    {
        case 32:
            weightFactorMatrix = weights_dct32;
            break;
        case 16:
            weightFactorMatrix = weights_dct16;
            break;
        case 8:
            weightFactorMatrix = weights_dct8;
            break;
    }

    for (unsigned i = 0; i < blockSize * blockSize; i++)
    {
        auto weightedCoeff = (uint32_t)(weightFactorMatrix[i] * std::abs(coeffBuffer[i]));
        weightedSum += weightedCoeff;
    }

    return weightedSum;
}

void copyPixelValuesToBuffer(unsigned blockOffsetLuma,
                             unsigned blockSize,
                             unsigned bitDepth,
                             uint8_t *src,
                             unsigned srcStride,
                             int16_t *buffer)
{
    if (bitDepth == 8)
    {
        src += blockOffsetLuma;
        for (unsigned y = 0; y < blockSize; y++, src += srcStride)
            for (unsigned x = 0; x < blockSize; x++)
                *(buffer++) = int16_t(src[x]);
    }
    else
    {
        auto input = (int16_t *) (src) + blockOffsetLuma;
        for (unsigned y = 0; y < blockSize; y++, input += srcStride / 2, buffer += blockSize)
            std::memcpy(buffer, input, blockSize * sizeof(int16_t));
    }
}

template<int bitDepth>
void copyPixelValuesToBufferWithPadding(unsigned blockOffsetLuma,
                                        unsigned blockSize,
                                        uint8_t *srcData,
                                        unsigned srcStride,
                                        int16_t *buffer,
                                        unsigned paddingRight,
                                        unsigned paddingBottom)
{
    typedef typename std::conditional<bitDepth == 8, uint8_t *, int16_t *>::type InValueType;
    static_assert(bitDepth >= 8 && bitDepth <= 16);

    const auto *__restrict src = InValueType(srcData);

    src += blockOffsetLuma;
    unsigned y          = 0;
    auto bufferLastLine = buffer;
    for (; y < blockSize - paddingBottom; y++, src += srcStride)
    {
        unsigned x     = 0;
        bufferLastLine = buffer;
        for (; x < blockSize - paddingRight; x++)
            *(buffer++) = int16_t(src[x]);
        const auto lastValue = int16_t(src[x]);
        for (; x < blockSize; x++)
            *(buffer++) = lastValue;
    }
    for (; y < blockSize; y++)
    {
        for (unsigned x = 0; x < blockSize; x++)
            *(buffer++) = bufferLastLine[x];
    }
}

void performDCT(unsigned blockSize, int16_t *pixelBuffer, int16_t *coeffBuffer)
{
    // DCT
    switch (blockSize)
    {
        case 32:
            vca::dct32_c(pixelBuffer, coeffBuffer, 32);
            break;
        case 16:
            vca::dct16_c(pixelBuffer, coeffBuffer, 16);
            break;
        case 8:
            vca::dct8_c(pixelBuffer, coeffBuffer, 8);
            break;
        default:
            throw std::invalid_argument("Invalid block size " + std::to_string(blockSize));
    }
}

} // namespace

namespace vca {

void computeWeightedDCTEnergy(const Job &job, Result &result, unsigned blockSize)
{
    const auto frame = job.frame;
    if (frame == nullptr)
        throw std::invalid_argument("Invalid frame pointer");

    const auto bitDepth = frame->info.bitDepth;

    if (frame->info.bitDepth > 8)
        throw std::invalid_argument("16 bit input not implemented yet");

    auto src       = frame->planes[0];
    auto srcStride = frame->stride[0];

    auto [widthInBlocks, heightInBlock] = getFrameSizeInBlocks(blockSize, frame->info);
    auto totalNumberBlocks              = widthInBlocks * heightInBlock;
    auto widthInPixels                  = widthInBlocks * blockSize;
    auto heightInPixels                 = heightInBlock * blockSize;

    if (result.energyPerBlock.size() < totalNumberBlocks)
        result.energyPerBlock.resize(totalNumberBlocks);

    // First, we will copy the source to a temporary buffer which has one int16_t value
    // per sample.
    //   - This may only be needed for 8 bit values. For 16 bit values we could also
    //     perform this directly from the source buffer. However, we should check the
    //     performance of that approach (i.e. the buffer may not be aligned)

    ALIGN_VAR_32(int16_t, pixelBuffer[32 * 32]);
    ALIGN_VAR_32(int16_t, coeffBuffer[32 * 32]);

    auto blockIndex       = 0u;
    uint32_t frameTexture = 0;
    for (unsigned blockY = 0; blockY < heightInPixels; blockY += blockSize)
    {
        auto paddingBottom = std::max(int(blockY + blockSize) - int(frame->info.height), 0);
        for (unsigned blockX = 0; blockX < widthInPixels; blockX += blockSize)
        {
            auto paddingRight    = std::max(int(blockX + blockSize) - int(frame->info.width), 0);
            auto blockOffsetLuma = blockX + (blockY * srcStride);

            if (paddingRight > 0 || paddingBottom > 0)
            {
                if (bitDepth == 8)
                    copyPixelValuesToBufferWithPadding<8>(blockOffsetLuma,
                                                          blockSize,
                                                          src,
                                                          srcStride,
                                                          pixelBuffer,
                                                          unsigned(paddingRight),
                                                          unsigned(paddingBottom));
                else
                    copyPixelValuesToBufferWithPadding<16>(blockOffsetLuma,
                                                           blockSize,
                                                           src,
                                                           srcStride / 2,
                                                           pixelBuffer,
                                                           unsigned(paddingRight),
                                                           unsigned(paddingBottom));
            }
            else
            {
                copyPixelValuesToBuffer(blockOffsetLuma,
                                        blockSize,
                                        bitDepth,
                                        src,
                                        srcStride,
                                        pixelBuffer);
            }

            performDCT(blockSize, pixelBuffer, coeffBuffer);

            result.energyPerBlock[blockIndex] = calculateWeightedCoeffSum(blockSize, coeffBuffer);
            frameTexture += result.energyPerBlock[blockIndex];

            blockIndex++;
        }
    }

    auto frameSizeInPixels = widthInPixels * heightInPixels;
    result.averageEnergy   = uint32_t((double)(frameTexture) / (totalNumberBlocks * E_norm_factor));
}

void computeTextureSAD(Result &result, const Result &resultsPreviousFrame)
{
    if (result.energyPerBlock.size() != resultsPreviousFrame.energyPerBlock.size())
        throw std::out_of_range("Size of energy result vector must match");

    auto totalNumberBlocks = result.energyPerBlock.size();
    if (result.sadPerBlock.size() < totalNumberBlocks)
        result.sadPerBlock.resize(totalNumberBlocks);

    double textureSad = 0.0;
    for (size_t i = 0; i < totalNumberBlocks; i++)
    {
        result.sadPerBlock[i] = uint32_t(
            std::abs(int(result.energyPerBlock[i]) - int(resultsPreviousFrame.energyPerBlock[i])));
        textureSad += result.sadPerBlock[i];
    }

    result.sad = textureSad / (totalNumberBlocks * h_norm_factor);
}

} // namespace vca