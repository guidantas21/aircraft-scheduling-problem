# Airport Scheduling Problem (ASP)

Solving the Airport Scheduling Problem (ASP) with independent runways.

## TODO

- [x] Instance parser
- [ ] Feasibility check
- [ ] Constructive procedure
- [ ] Local search
- [ ] Methaheuristic

## Methaheuristics

- [ ] ILS (Iterated Local Search)
- [ ] GRASP (Greedy Randomized Adaptative Search Procedure)

## Constructive procedure

- [ ] Cheapest Insertion
- [ ] Other

## Local search

- [ ] VND (Variable Neighborhood Search) [MANDATORY]
- [ ] RVND (Randomized Variable Neighborhood Search)

## Getting started

### Prerequisites

- [Meson Build System](https://mesonbuild.com/)
- C++ 17

### How to build the project

1. Setup release and debug builds:

```
meson setup build --buildtype=release
meson setup build_debug --buildtype=debug
```

2. Compile build:

```
meson compile -C <build or build_debug>
```

3. Run:

From the root directory:

```
./<build or build_debug>/src/asp <instance file path>
```

## How to contribute

1. Create a branch with a name that describes the feature added:

```git checkout -b <branch-name>```

2. After the implementation of the feature, commit your changes with a [semmantic commit message](https://www.conventionalcommits.org/en/v1.0.0/):

```
git add .
git commit -m "feat: <changes made>"
```

3. Then push local changes to the remote repository:

```
git push origin <branch-made>
```

4. In the remote repository, create a [pull request](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/about-pull-requests).
