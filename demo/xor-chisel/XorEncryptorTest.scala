// file: XorEncryptorTest.scala
import chisel3._
import chiseltest._
import org.scalatest.flatspec.AnyFlatSpec

/**
 * XorEncryptor 模块的测试平台，使用 chiseltest 库。
 * 模拟了 Verilog 测试平台中的测试序列。
 */
class XorEncryptorTest extends AnyFlatSpec with ChiselScalatestTester {
  "XorEncryptor" should "perform XOR encryption correctly" in {
    // 实例化模块并启动仿真
    test(new XorEncryptor()).withAnnotations(Seq(WriteVcdAnnotation)) { c =>
      // 1. 复位
      println("----------------------------------------")
      println("Test started.")
      
      c.io.rst_n.poke(false.B)
      c.io.start.poke(false.B)
      c.io.data_in.poke(0.U)
      c.io.key_in.poke(0.U)
      c.clock.step(1) // 步进一个时钟周期以完成复位

      c.io.rst_n.poke(true.B)
      println("复位完成。")

      // 2. 第一次加密操作
      val data_1 = "hdeadbeef".U(32.W)
      val key_1  = "h12345678".U(32.W)
      val expected_1 = data_1 ^ key_1

      println(s"Time=${c.clock.get, "Starting encryption with:"}")
      println(s"Data_in = ${data_1}")
      println(s"Key_in  = ${key_1}")

      c.io.data_in.poke(data_1)
      c.io.key_in.poke(key_1)
      c.io.start.poke(true.B)
      c.clock.step(1)
      c.io.start.poke(false.B)
      
      // 等待状态机从 ENCRYPTING 到 DONE
      c.clock.step(1) 
      
      // 3. 验证结果
      println(s"Time=${c.clock.get, "Encryption finished."}")
      c.io.done.expect(true.B) // 检查 done 信号
      c.clock.step(1)
      c.io.done.expect(false.B) // 检查 done 信号是否在一个周期后拉低
      
      println(s"Expected output = ${expected_1}")
      println(s"Actual output   = ${c.io.data_out.peek()}")

      // 检查密文输出
      c.io.data_out.expect(expected_1)
      println("----------------------------------------")
      println("Test PASSED!")

      // 4. 第二次加密操作
      val data_2 = "h01234567".U(32.W)
      val key_2  = "habcdef01".U(32.W)
      val expected_2 = data_2 ^ key_2
      
      c.io.data_in.poke(data_2)
      c.io.key_in.poke(key_2)
      c.io.start.poke(true.B)

      println("----------------------------------------")
      println("Test another case:")
      println(s"Time=${c.clock.get, "Starting encryption with:"}")
      println(s"Data_in = ${data_2}")
      println(s"Key_in  = ${key_2}")

      c.clock.step(1)
      c.io.start.poke(false.B)
      
      // 等待状态机从 ENCRYPTING 到 DONE
      c.clock.step(1) 

      // 验证第二次结果
      c.io.done.expect(true.B)
      c.clock.step(1)
      
      println(s"Time=${c.clock.get, "Encryption finished."}")
      println(s"Expected output = ${expected_2}")
      println(s"Actual output   = ${c.io.data_out.peek()}")
      
      c.io.data_out.expect(expected_2)
      println("----------------------------------------")
      println("Test PASSED!")
    }
  }
}
