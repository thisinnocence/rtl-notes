// file: XorEncryptor.scala
import chisel3._
import chisel3.util._

/**
 * 一个简单的 32 位 XOR 对称加密时序电路模块，使用 Chisel 实现。
 * 该模块包含一个简单的状态机来控制加密流程。
 */
class XorEncryptor extends Module {
  // 定义输入/输出接口
  val io = IO(new Bundle {
    // Verilog 的 clk 和 rst_n 在 Chisel 中是隐式的，这里我们只定义 rst_n 的逻辑。
    // Chisel 默认使用同步高电平复位，但我们可以通过 withReset 模拟低电平复位。
    val rst_n   = Input(Bool()) // 同步低电平复位信号
    val start   = Input(Bool()) // 启动加密操作的信号 (脉冲)
    val data_in = Input(UInt(32.W)) // 32 位明文数据输入
    val key_in  = Input(UInt(32.W)) // 32 位密钥输入
    val data_out= Output(UInt(32.W)) // 32 位密文数据输出
    val done    = Output(Bool())    // 加密完成指示信号 (脉冲)
  })

  // 使用 withReset 来模拟 Verilog 的异步低电平复位行为
  withReset(!io.rst_n) {
    // 定义状态机
    val sIDLE :: sENCRYPTING :: sDONE :: Nil = Enum(3)
    // 状态寄存器，默认复位到 sIDLE
    val state = RegInit(sIDLE)

    // 内部寄存器，用于锁存输入数据和密钥
    val currentData = RegInit(0.U(32.W))
    val currentKey  = RegInit(0.U(32.W))

    // 输出寄存器，用于存储密文和 done 信号
    val data_out_reg = RegInit(0.U(32.W))
    val done_reg     = RegInit(false.B)

    // 默认输出值
    io.data_out := data_out_reg
    io.done     := done_reg

    // 状态机逻辑
    switch(state) {
      is(sIDLE) {
        // 在 IDLE 状态，done 信号为低
        done_reg := false.B
        when(io.start) {
          // 如果接收到启动信号，锁存输入数据和密钥
          currentData := io.data_in
          currentKey  := io.key_in
          // 进入加密状态
          state := sENCRYPTING
        }
      }
      is(sENCRYPTING) {
        // 执行 XOR 加密操作，并将结果赋给输出寄存器
        // 这将在下一个时钟周期输出结果
        data_out_reg := currentData ^ currentKey
        done_reg     := true.B // 加密完成，拉高 done 信号
        state        := sDONE   // 进入完成状态
      }
      is(sDONE) {
        // 仅在 ENCRYPTING 状态的下一个周期拉高 done，然后立即拉低
        done_reg := false.B
        state    := sIDLE // 返回空闲状态，等待下一次启动
      }
    }
  }
}
