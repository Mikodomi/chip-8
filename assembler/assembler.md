# Instructions and their corresponding opcodes

| Opcode | Instruction |
|--------|-------------|
| `00E0` | `clear`     |
| `00EE` | `ret`       |
| `0nnn` | `jnat nnn`  |
| `1nnn` | `jmp nnn`   |
| `2nnn` | `call nnn`  |
| `3Xnn` | `skpe vX, nn`|
| `4Xnn` | `skpne vX, nn`|
| `5XY0` | `skpe vX, vY` |
| `6Xnn` | `mov vX, nn` |
| `7Xnn` | `add vX, nn` |
| `8XY0` | `mov vX, vY` |
| `8XY1` | `or vX, vY` |
| `8XY2` | `and vX, vY` |
| `8XY3` | `xor vX, vY` |
| `8XY4` | `add vX, vY` |
| `8XY5` | `sub vX, vY` |
| `8XY6` | `msr vX, vY` |
| `8XY7` | `subs vX, vY` |
| `8XYE` | `msl vX, vY` |
| `9XY0` | `skpne vX, vY` |
| `Annn` | `mov I, nnn` |
| `Bnnn` | `jv0 nnn`   |
| `CXnn` | `rand vX, nn` |
| `DXYn` | `draw vX, vY, n` |
| `EX9E` | `skpe vX` |
| `EXA1` | `skpne vX` |
| `FX07` | `mov vX, dtm` |
| `FX0A` | `in vX` |
| `FX15` | `mov dtm, vX` |
| `FX18` | `mov stm, vX` |
| `FX1E` | `add I, vX` |
| `FX29` | `hh5 vX` |
| `FX33` | `bcd vX` |
| `FX55` | `movout vX` |
| `FX65` | `movin vX` | 
### Note
- `nnn` - memory address
- `nn, n` - constant numeric value
- `vX` - the Xth register (X must be hexadecimal)
- `dtm, stm` - Delay timer, Sound timer
- `I` - the special address register

Memory addresses and numeric values can be written in binary (`nnnB`), octal (`nnnO`), decimal (no suffix) or hexadecimal (`nnnH`) format.



