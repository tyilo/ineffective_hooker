ineffective_hooker
==================

Similar to [mach_override](https://github.com/rentzsch/mach_override)

Lets you replace a functions implementation with another function which can call the original function normally.

Example usage
-------------

```
#include <stdio.h>
#include "hooker.h"

int foo(void) {
	return 1;}

int my_foo(void) {
	return foo() + 1;}

int main(void) {
	printf("%d\n", foo()); // 1
	hook(foo, my_foo, NULL, NULL);
	printf("%d\n", foo()); // 2}
```

Using other hooking libraries such as [CydiaSubstrate](http://www.cydiasubstrate.com/) or mach_override you would get back a reference to the original function which then must be used inside the replacement function.

This is not required (or possible) with ineffecitive_hooker.

How it works
------------

ineffecitive_hooker works by replacing the original function with some self-modifying code that does the following every time the original function is called:

- puts the original function's code back in place
- calls the replacement function (which might call the original function)
- puts the hooking code back instead of the original function

The reason this code is so ineffective is that instead of requiring a few more instructions to be executed like other libraries, it makes the code run a couple of thousand instruction more instructions instead.