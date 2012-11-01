#!/bin/sh
#
# This file is designed to make sure we run all existing
# unit tests in each of the new modes to ensure that existing
# functionality still continues to work.
#
# Example usage:
#
# ./gen_tests.sh
# pear run-tests
# ./cleanup_tests.sh

echo "Generating return null and throw exceptions tests"

for i in [og]*.phpt 
do 
  # We want to exclude any tests that are already testing specific 
  # behavior relating to reduce_fatals
  grep -q "odus.reduce_fatals" $i
  if [ $? == 1 ] 
  then 
     # [1] Create tests to run with "null return" behavior
     cp $i null_$i 
     grep -q "\-\-INI\-\-" null_$i 
     # If there isn't already a --INI-- block then add one
     if [ $? == 1 ] 
     then 
       echo "--INI--" >> null_$i 
     fi 
     sed -i 's/INI--/INI--\nodus.reduce_fatals=1/' null_$i 

     # [2] Create tests to run with throw exceptions behavior
     sed 's/INI--/INI--\nodus.throw_exceptions=1/' null_$i > exception_$i 
  fi 
done