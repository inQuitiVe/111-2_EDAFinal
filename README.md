# 111-2_EDAFinal

Files In The Zip File:
    - this README file
    - slides
    - report
    - source codes (cases are too large to be included)
        - main.cpp (main program)
        - dmpTopl.cpp (transfer dmp file to pl file)

How To Compile:
g++ main.cpp -o DMP
g++ dmpTopl.cpp -o tran

How To Run:
./DMP caseOO.v caseOO.lef caseOO.def caseOO.mlist caseOO.txt caseOO.dmp
./tran caseOO.dmp caseOO.pl caseOO_out.pl
