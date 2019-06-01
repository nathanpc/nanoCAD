# nanoCAD Command List

This is the official documentation of all of the available commands for nanoCAD. Some might be missing because I've implemented them and forgot to document them here.


## Basics

Some basic things you should know to understand this documentation.

A nanoCAD file is literally just a sequence of commands as if it was the history of the command line in a regular CAD program.

All objects are represented by their *identification symbol*, which are as follows:

  - `@` for Coordinates
  - `$` for Numbers
  - `&` for Objects
  - `l` for Layers

Coordinates are represented in the form of `x<num>;y<num>` and can have units attached to them like `x1.5m;y40cm`.

Some commands can take *modifier arguments* that are in the form of `c<something>` where `c` is a character that determines which type of modifier it is (a common one is `l` for layer) and `something` is the modifier value that can be anything. For example if we want to put something in layer number 3 the documentation would be like `l<$layer_num>` and you would write it as `l3` in the command argument.


## Primitive Objects

The objects that should be the basis to form your drawings.

### Lines

Lines can be from point A to point B or can be from a point and have a width or height specified by the `$distance` argument.

  - `line @start, @end, [l<$layer_num>]`: Draws a line from `@start` to `@end` in layer `$layer_num`.
    - `@start`: Line starting point.
	- `@end`: Line ending point.
	- `l<$layer_num>`: Layer number.
  - `line @start, w<$width>, [l<$layer_num>]`: Draws a line from `@start` with a width of `$width` in layer `$layer_num`.
    - `@start`: Line starting point.
	- `w<$width>`: Line width. Negative numbers will make the line go left from the starting point.
	- `l<$layer_num>`: Layer number.
  - `line @start, h<$height>, [l<$layer_num>]`: Draws a line from `@start` with a height of `$height` in layer `$layer_num`.
    - `@start`: Line starting point.
	- `h<$height>`: Line height. Negative numbers will make the line go down from the starting point.
	- `l<$layer_num>`: Layer number.

