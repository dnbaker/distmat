# distmat
2-dimensional distance matrix for holding distances of arbitrary types.

-----------

Note: for row == column, this will return a reference to a member-contained type. This allows you to dynamically set the default value for distance
between identical elements (IE, an object and itself).

This requires C++14 and unistd.h (typically POSIX compliance).

===========

### Python bindings

Experimental python bindings are available for float matrices only.

```
from distmat import dm
mat = dm(100)
v1 = .4
mat(0, 3) = v1
assert mat(3, 0) == v1
mat.write("mat.dm")
copied = dm("mat.dm")  # copied now has the same contents as mat.
```
