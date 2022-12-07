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

# Calls check_count.awk on the testrun files and writes the output in a .res file.
#
# To be invoked by 'check_count.sh'.

export LANG=C

AWKARGS=""
FILES=""
for i in $@
do
    if test ! -e ${i}
    then
        AWKARGS="${AWKARGS} ${i}"
    else
        FILES="${FILES} ${i}"
    fi
done

for i in ${FILES}
do
    NAME=$(basename ${i} .out)
    DIR=$(dirname ${i})
    OUTFILE="${DIR}/${NAME}.out"
    RESFILE="${DIR}/${NAME}.res"
    TEXFILE="${DIR}/${NAME}.tex"
    PAVFILE="${DIR}/${NAME}.pav"

    TSTNAME=$(echo "${NAME}" | sed 's/checkcount.\([a-zA-Z0-9_]*\).*/\1/g')

    if test -f "testset/${TSTNAME}.test"
    then
        TESTFILE="testset/${TSTNAME}.test"
    else
        TESTFILE=""
    fi

    # call method to obtain solution file
    # defines the following environment variable: SOLUFILE
    . ./configuration_solufile.sh "${TSTNAME}"

    # the variable AWKARGS needs to be without quotation marks here
    awk -f check_count.awk ${AWKARGS} "${TESTFILE}" "${SOLUFILE}" "${OUTFILE}" | tee "${RESFILE}"
done
