#!/bin/bash
set -e

readonly test_name=$(basename -s .sh $0)
readonly root=$(realpath $(dirname $0)/..)
readonly hier_manip=${root}/vcd_hier_manip

rm -rf "${root}/test_run/${test_name}"
mkdir -p "${root}/test_run/${test_name}"

pushd "${root}/test_run/${test_name}" > /dev/null

cp -p ${root}/tests/${test_name}.vcd 0.vcd
cp -p 0.vcd 1.vcd
${hier_manip} 1.vcd
cp -p 1.vcd 2.vcd
${hier_manip} --flatten 2.vcd

if diff -w -B 0.vcd 2.vcd; then
    echo "Test ${test_name} Pass"
else
    echo "Test ${test_name} Fail"
    exit 1
fi

popd > /dev/null
