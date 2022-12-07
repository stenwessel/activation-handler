#!/usr/bin/env bash
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*                                                                           *
#*                  This file is part of the program and library             *
#*         SCIP --- Solving Constraint Integer Programs                      *
#*                                                                           *
#*    Copyright (C) 2002-2021 Konrad-Zuse-Zentrum                            *
#*                            fuer Informationstechnik Berlin                *
#*                                                                           *
#*  SCIP is distributed under the terms of the ZIB Academic License.         *
#*                                                                           *
#*  You should have received a copy of the ZIB Academic License              *
#*  along with SCIP; see the file COPYING. If not email to scip@zib.de.      *
#*                                                                           *
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

# Generate a comparison of two testruns, i.e. with different settings.
#
# Usage: 'allcmpres.sh check.run1.res check.run2.res'
#        'allcmpres.sh check.run1.*.res'

AWKARGS=""
FILES=""
for i in $@
do
    if test ! -e "${i}"
    then
        AWKARGS="${AWKARGS} ${i}"
    else
        FILES="${FILES} ${i}"
    fi
done

TESTSETS=""
for i in $(ls -1 ${FILES} | sed 's!\(.*\)check\.\([^ .]*\)\.\([^ ]*\)\.res!\2!g' | sort -u)
do
    TESTSETS="${TESTSETS} ${i}"
done

export LC_NUMERIC=C

for i in ${TESTSETS}
do
    echo
    echo "====vvvv==== ${i} ====vvvv===="
    # the variable AWKARGS needs to be without quotation marks here
    awk -f cmpres.awk ${AWKARGS} texcmpfile="cmpres.${i}.tex" $(ls -1f ${FILES} | grep "${i}\..*\.res")
    echo "====^^^^==== ${i} ====^^^^===="
done
