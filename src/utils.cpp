
#include "hdf5.h"

bool is_any_real_type(hid_t type_id)
{
    if (
        H5Tequal(type_id, H5T_NATIVE_FLOAT) ||
        H5Tequal(type_id, H5T_NATIVE_DOUBLE) ||
        H5Tequal(type_id, H5T_NATIVE_LDOUBLE))
    {
        return true;
    }
    return false;
}

bool is_any_integer_type(hid_t type_id)
{
    if(H5Tequal(type_id,  H5T_NATIVE_CHAR) ||
        H5Tequal(type_id, H5T_NATIVE_UCHAR) ||
        H5Tequal(type_id, H5T_NATIVE_SHORT) ||
        H5Tequal(type_id, H5T_NATIVE_USHORT) ||
        H5Tequal(type_id, H5T_NATIVE_INT) ||
        H5Tequal(type_id, H5T_NATIVE_UINT) ||
        H5Tequal(type_id, H5T_NATIVE_LONG) ||
        H5Tequal(type_id, H5T_NATIVE_ULONG) ||
        H5Tequal(type_id, H5T_NATIVE_LLONG) ||
        H5Tequal(type_id, H5T_NATIVE_ULLONG))
    {
        return true;
    }
    return false;
}
