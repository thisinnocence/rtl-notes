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

#### 安装 scala 环境

- 安装JDK: `sudo apt install default-jdk`
- 安装scala: <https://www.scala-lang.org/download/3.3.6.html>, 直接安装github归档的二进制包即可;

    ```bash
    wget https://github.com/scala/scala3/releases/download/3.3.6/scala3-3.3.6.tar.gz
    ```

- 安装SBT: <https://www.chisel-lang.org/docs/installation#linux>, 直接下载tar包解压即可；

上面的 scala 和 sbt 安装完毕后，可以参考伯克利的教学项目测试。

#### 配置阿里maven源

`vim ~/.sbt/repositories`，内容：

```ini
[repositories]
  local
  aliyun: https://maven.aliyun.com/repository/public
  central: https://repo1.maven.org/maven2/
```

#### Chisel项目测试

教学仓库： <https://github.com/ucb-bar/chisel-tutorial>
