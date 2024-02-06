#include <engine/utils/hashmap.h>

/* Currently, this is a "works, but very poorly" placeholder implementation.
 * Should be avoided in practice */
i32 lolhash(const usize s, i32 v) { return v % s; }
