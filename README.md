# rtl-notes

Chip design learn.

## 环境准备

### iverilog

```bash
sudo apt-get install iverilog
sudo apt-get install gtkwave
```

其中，gtkwave 是波形查看工具，WLS2 里可以直接打开调出来GUI。
也可以使用 vscode 插件 WaveTrace 直接看。

### SystemC

```bash
sudo apt-get install libsystemc-dev
```

或者源码安装

```bash
git clone https://github.com/accellera-official/systemc.git
cd systemc
mkdir build
cd build
../configure   # 也可以加 --prefix=your_path 指定安装路径, 不加就默认安装到当前仓库根目录
make -j
make install   # 可以安装到当前目录
```

对于 C/C++ 项目， `make` 的时候加上 `bear -- make` 可以生成 `compile_commands.json` 方便 vscode-clangd 智能提示。

### ChiSel

- 安装JDK: `sudo apt install default-jdk`
- 安装scala: <https://docs.scala-lang.org/getting-started/install-scala.html>
- 构建SBT(Scala Build Tool)：<https://www.scala-sbt.org/1.x/docs/Installing-sbt-on-Linux.html>
- 安装Chisel: <https://www.chisel-lang.org/docs/installation>

很多 RISC-V 开源项目用，这个适合入门参考:

- <https://github.com/schoeberl/chisel-examples>
- <https://github.com/chipsalliance/chisel-template>
- <https://github.com/ucb-bar/riscv-mini>
