/*
 * dt_enum.c: Enumerations for Unit 5, Section C.
 *
 * A C enumeration type is compatible with an integer type and uses named
 * enumerators. A dt_color can still hold 47. This module instead accepts only
 * the three declared color ordinals. Other languages place different
 * restrictions on creating enumeration values from arbitrary integers.
 *
 * These three functions validate each enumeration operation in one location.
 */

#include "dt.h"

#include <string.h>

static const char *const COLOR_NAMES[] = { "RED", "GREEN", "BLUE" };

/*
 * dt_enum_is_valid returns true for a declared ordinal. C permits any integer
 * in an enumeration object. This function validates the declared range.
 */
bool dt_enum_is_valid(int ordinal)
{
    return ordinal >=0 && ordinal < DT_COLOR_COUNT; //validates ordinals (blocks negatives and higher than the color count)

}

/*
 * dt_enum_name writes the enumerator text to *out. It returns DT_ERR_RANGE for
 * an invalid ordinal. A failure preserves *out.
 */
dt_status dt_enum_name(int ordinal, const char **out)
{
    if (!dt_enum_is_valid(ordinal)){
        return DT_ERR_RANGE; //returns error if invalid
    }
    *out = COLOR_NAMES[ordinal]; //writes to *out
    return DT_OK;
}

/*
 * dt_enum_from_name searches the enumerator text and writes its ordinal to
 * *out. It returns DT_ERR_RANGE when the text has no match.
 */
dt_status dt_enum_from_name(const char *name, int *out)
{
    for (int i = 0; i <DT_COLOR_COUNT; i++) { //loops through the names
        if (strcmp(name, COLOR_NAMES[i]) == 0){ //if match write the position/index to *out
            *out = i;
            return DT_OK;
        }
    }
    return DT_ERR_RANGE;
}
