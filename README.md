# Handling Sub-symmetry in Integer Programming using Activation Handlers

This repository contains the experiments setup for testing the activation handler approach for sub-symmetry handling in the Multiple Knapsack Problem, Muin-up/min-down Unit Commitment Problem and the Max-k Colorable Subgraph Problem.

## Installation
The following may be used as a reference to completely install a working environment. It assumes GCC, CMake and Python version (at least) 3.10 are already installed. It has been tested and known to work with GCC version 9.2.0 and CMake version 3.18.4. Note: change /path/to to a path of your liking.

```bash
# Install 3rd party dependencies

# Boost
wget https://boostorg.jfrog.io/artifactory/main/release/1.74.0/source/boost_1_74_0.tar.gz
tar -xf boost_1_74_0.tar.gz
rm boost_1_74_0.tar.gz

cd boost_1_74_0 || exit

./bootstrap.sh --prefix=../boost
./b2 install --prefix=../boost --with=all -j5

cd - || exit

# Bliss (for symmetry detection)
git clone --depth 1 --branch v0.73.3 git@github.com:ds4dm/Bliss.git

cd Bliss || exit
cmake .
cmake --build .

cd - || exit

# Soplex
git clone --depth 1 --branch release-600 git@github.com:scipopt/soplex.git

cd soplex || exit

cmake -DBOOST_INCLUDEDIR=../boost/include -DBOOST_LIBRARYDIR=../boost/lib -DBoost_NO_SYSTEM_PATHS=ON -DBOOST_ROOT=../boost -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./install-release -DPAPILO=off -B ./cmake-build-release
cmake --build ./cmake-build-release --clean-first --target install -- -j 5

cd - || exit

# SCIP
cd scip || exit

cmake -DCMAKE_BUILD_TYPE=Release -DPAPILO=off -DSOPLEX_DIR=../soplex/cmake-build-release -DZIMPL=off -DIPOPT=off -DBLISS_DIR=../Bliss -DCMAKE_INSTALL_PREFIX=./install-release -B ./cmake-build-release
cmake --build ./cmake-build-release --clean-first --target install -- -j 5

cd - || exit

# Create venv
python3.10 -m venv venv
source ./venv/bin/activate

pip install -r experiments/requirements.txt

# Install pyscipopt
export SCIPOPTDIR="/path/to/scip/install-release"
pushd pyscipopt || exit
ln -s src/pyscipopt pyscipopt
pip install -e .
popd
```

## Instances
Instances for the MKP are included in the `experiments/tests/data` directory.
The Color02 instances for the MKCS problem can be found [here](https://mat.tepper.cmu.edu/COLOR02/).
The MUCP instances are available [here](https://github.com/CecileRottner/SymmetryBreakingIneq).

## Running the experiments
Tests can be run in the `experiments/tests` directory, by running
```bash
export SCIPOPTDIR="/path/to/scip/install-release"
export PYTHONPATH="../src"
python runner_{mkp,mucp,mkcs_orig}.py <id>
```
where `<id>` is substituted by the index of the instance to solve (see `experiments/tests/index/{mkp,mucp,mkcs}`).
Make sure the Python virtual environment is 'activated'.

## Licensing
The SCIP version used is licenced under the ZIB Academic License.
All code in the `scip` directory is published under this license.

PySCIPOpt and the experiments source code is licensed under the MIT license.
