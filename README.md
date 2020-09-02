# RISC V Simulation Software
This is a simulation software for RISC-V(32 bit)! It converts assembly instructions to machine instructions. It can be implemented with\without pipeline. It supports forwarding as well as stalling. Cache support is also included along with branch prediction.


## Directories :
```bash
 |
  |- src
      |- assembler.cpp
      |- unPipelinedSimulator.cpp
      |- simulator.cpp
  |- Assembly Codes
      |
      |- fibo_rec.asm
```


## Run Assembler :
In terminal go to file location :

```bash
    g++ assembler.cpp -o ./a
    ./a
```

Now enter the file name like assemblyCode.asm or fibo_rec.asm or any other!


## Run Simulator :
```bash
    g++ simulator.cpp -o ./b
    ./b
 ```
 Choose accordingly!
 Results will be stored in regFile.txt, stats.txt, memory.txt
 
 
## Overview :

- assembler             : Converts assembly code to machine code
- unPipelinedSimulator  : Unpipelined simulation of machine code
- simulator             : Pipelined/(Unpipelined) Stalling, Forwarding, Static Branch Prediction, Cache Support 
- Branch Prediction     : Static & Always Not Taken
- Misprediction Penalty : 2 stalls
- Cache                 : Direct Map, Set Assosiative, Fully Associative


