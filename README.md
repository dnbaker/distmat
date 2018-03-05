# distmat
2-dimensional distance matrix for holding distances of arbitrary types.

-----------

Note: for row == column, this will return a reference to a member-contained type. This allows you to dynamically set the default value for distance
between identical elements (IE, an object and itself).

This requires C++17 and unistd.h (typically POSIX compliance).
