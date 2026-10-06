<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/images/smarttables-logo-white.svg">
  <img src="docs/images/smarttables-logo.svg" alt="Smart Tables" width="300">
</picture>

Smart Tables is the table widget UMG does not ship: a million virtualised rows, columns declared in one
place so no row can disagree with the header, and two-level natural sort that runs off the game thread.
Rows come from an array of UObjects, from a DataTable, or from a model of your own, and the first two
need no C++ at all. It is a commercial plugin for Unreal Engine 5.8, and this repository is a reading
copy of its source, published as a portfolio piece.

## The documentation

Everything is written down there: how a table is built, what each panel field does, the nine demos, and
a reference page for every public header.

**https://riztazz.github.io/smart-tables/**

## About this repository

Every comment is stripped from the source here, which takes the doc comments with them, and with those
the Blueprint node and Details panel tooltips UHT bakes from them. The documented source ships with the
plugin on FAB.

Not licensed for use, see `LICENSE`. Buy it on FAB to actually use it:
https://www.fab.com/listings/c2cddc8e-0844-41bc-b58d-9cedd12f71fd
