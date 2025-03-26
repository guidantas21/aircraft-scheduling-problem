# Airport Scheduling Problem (ASP)

Solving the Airport Scheduling Problem (ASP) with independent runways.

## Methaheuristics

Options:

[] ILS (Iterated Local Search)
[] GRASP (Greedy Randomized Adaptative Search Procedure)

## Constructuve procedure

[] Cheapest Insertion
[] Other

## Local search

Options:

[v] VND (Variable Neighborhood Search) [MANDATORY]
[] RVND (Randomized Variable Neighborhood Search)

## How to run

### Requirements

- [Meson Build System]()
- C++17

### Release build

1. Setup:

```
meson setup build --buildtype=release
```

2. Compile:

```
meson compile -C build
```

3. Run:

From the root directory:

```
./build/src/asp
```

### Debug build

1. Setup:

```
meson setup build_debug --buildtype=debug
```

2. Compile:

```
meson compile -C build_debug
```

3. Run:

From the root directory:

```
./build_debug/src/asp
```


## How to contribute

1. Create a branch with a name that describes the feature added:

```git checkout -b <branch-name>```

2. After the implementation of the feature, commit your changes with a [semmantic commit message]():

```
git add .
git commit -m "feat: <changes made>"
```

3. Then push local changes to the remote repository:

```
git push origin <branch-made>
```

4. In the remote repository, create a pull request.
