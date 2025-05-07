# Airport Scheduling Problem (ASP)

This project solves the Airport Scheduling Problem (ASP) with independent runways, focusing on optimizing aircraft scheduling to improve efficiency and minimize conflicts.

🏆 Winner of the Copa APA 2024.2 (Team "Prova Surpesa (01/04)")
A competition held between 27 undergraduate students from the Computer Science Center of the Federal University of Paraíba (CI/UFPB), where our algorithm won 1st place!

![Competition Ranking](./docs/ranking.png)

## TODO

- [x] Instance parser
- [x] Feasibility check
- [x] Constructive procedure
- [x] Local search
- [x] Perturbation
- [x] Methaheuristic

## Methaheuristics

- [x] GRASP (Greedy Randomized Adaptative Search Procedure)
- [x] GILS (GRASP Iterated Local Search)

## Constructive procedure

- [x] Cheapest Insertion
- [x] Other

## Local search

- [x] VND (Variable Neighborhood Search) [MANDATORY]
- [x] RVND (Randomized Variable Neighborhood Search)

### Neighborhood

- [x] INTRA-SWAP: swap two flights in the same runway
- [x] INTER-SWAP: swap two flights in different runways
- [x] INTRA-MOVE: move flight to different position in the same runway
- [x] INTER-MOVE: move flight to a different runway

### Perturbation

- [x] RANDOM-INTER-SWAP: swap two random blocks of flights in different runways 

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
