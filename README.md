## ApexSimulator
- Developed a simple 7 stage pipeline simulator handling in-order processing of a basic instruction set built with C.
- The input file is parsed token by token, delimited by commas and then stored in memory by copying the given input operands and instruction opcode into the corresponding registers. 
- Based on opcode, instructions move along the pipeline through latches where memory instructions are sent to the memory units and other instructions are sent to the execution units.
- This simulator consists of two execution and two memory units to provide for a fairly good amount of instruction level parallelism.
