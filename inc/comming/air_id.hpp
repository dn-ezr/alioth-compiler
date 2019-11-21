#ifndef __air_id__
#define __air_id__

namespace alioth::air {

using id_t = int;

/**
 * @constants : 全局掩码，区分ID的类型
 */
constexpr id_t TypeIdMask           = 0x10'00'00'00; // 类型掩码
constexpr id_t InstIdMask           = 0x20'00'00'00; // 指令掩码

/**
 * @constants : 类型掩码，用于对类型ID分组
 */
constexpr id_t IntTypeMask          = 0x00'01'00'00 | TypeIdMask; // 整数类型掩码
constexpr id_t FloatTypeMask        = 0x00'02'00'00 | TypeIdMask; // 浮点数类型掩码

/**
 * @constants : 类型ID,用于快速确认一个类型的情况
 * @desc :
 *  AIR在类型层面不区分有无符号，区分有无符号的数据的运算的工作由高级语言进行处理
 *  对AIR来说，并不存在数组数据类型，数组是全局变量的特征属性
 *  虽然我们也可以在局部空间开辟数组，并将数组地址的地址交付给一个Value，但此时只是使用了数组分配变量
 *  数组首地址的地址是指，虽然没有数组数据类型，但是我们将整个数组视为一个对象，将它的首地址交付，所以又多了一层指针层次
 *  但是，实际上没有更多的影响
 */
constexpr id_t VoidTypeId           = 0x00'00'00'01 | TypeIdMask; // 空类型，不能直接用作符合数据类型成员，不能用于定义变量
constexpr id_t Int1TypeId           = 0x00'00'00'02 | IntTypeMask; // 1位整数类型，实际机器不一定拥有此数据类型，主要用于表示一个数字中只有一个位有效，被分支指令用作条件
constexpr id_t Int8TypeId           = 0x00'00'00'03 | IntTypeMask; // 8位整数类型
constexpr id_t Int16TypeId          = 0x00'00'00'04 | IntTypeMask; // 16位整数类型
constexpr id_t Int32TypeId          = 0x00'00'00'05 | IntTypeMask; // 32位整数类型
constexpr id_t Int64TypeId          = 0x00'00'00'06 | IntTypeMask; // 64位整数类型
// [暂不支持] constexpr id_t Int128TypeId      = 0x07 | IntTypeMask; // 128位整数类型，有可能对应于sse扩展指令集中的扩展数据包
// [暂不支持] constexpr id_t Int256TypeId      = 0x08 | IntTypeMask; // 256位整数类型，有可能对应于sse扩展指令集中的扩展数据包
constexpr id_t Float32TypeId        = 0x00'00'00'09 | FloatTypeMask; // IEEE754格式的短浮点类型
constexpr id_t Float64TypeId        = 0x00'00'00'0A | FloatTypeMask; // IEEE754格式的长浮点类型
constexpr id_t Poid_terTypeId       = 0x00'00'00'0B | TypeIdMask; // 指针数据类型
constexpr id_t CompositeTypeId      = 0x00'00'00'0C | TypeIdMask; // 复合数据类型，此类型没有常量，使用结构体表达式构造此类型的全局变量
constexpr id_t FunctionTypeId       = 0x00'00'00'0D | TypeIdMask; // 函数数据类型，不能用作变量类型，可以作为指针类型的子类型
constexpr id_t BlockEntryTypeId     = 0x00'00'00'0E | TypeIdMask; // 块入口地址类型，仅用于为Block提供类型支持

/**
 * @constants : 指令掩码，用于对指令进行分类
 */
constexpr id_t TermInstMask         = 0x00'01'00'00 | InstIdMask; // 终结指令
constexpr id_t UnaryOpInstMask      = 0x00'02'00'00 | InstIdMask; // 单目运算指令
constexpr id_t BinaryOpInstMask     = 0x00'03'00'00 | InstIdMask; // 双目运算指令
constexpr id_t MemoryOpInstMask     = 0x00'04'00'00 | InstIdMask; // 内存操作指令
constexpr id_t CastOpInstMask       = 0x00'05'00'00 | InstIdMask; // 转换操作指令
constexpr id_t OtherInstMask        = 0x00'06'00'00 | InstIdMask; // 其他指令


/**
 * @constants : 指令ID
 */
constexpr id_t RetInstId            = 0x00'00'00'01 | TermInstMask; // 返回指令，有返回值或无返回值
constexpr id_t BrInstId             = 0x00'00'00'02 | TermInstMask; // 跳转指令，有条件或无条件
constexpr id_t SwitchInstId         = 0x00'00'00'03 | TermInstMask; // 选择指令
//[暂不支持] constexpr id_t IndirectBrInstId     = 0x00'00'00'04 | TermInstMask; // 间接跳转指令,跳转目标存放在数组里，所以目标地址可变
//[暂不支持] constexpr id_t InvokeInstId         = 0x00'00'00'05 | TermInstMask; // 普通/带异常调用
//[暂不支持] constexpr id_t ResumeInstId         = 0x00'00'00'06 | TermInstMask; // 抛出异常
//[暂不支持] constexpr id_t UnreachableInstId    = 0x00'00'00'07 | TermInstMask; // 
//[暂不支持] constexpr id_t CleanupRetInstId     = 0x00'00'00'08 | TermInstMask; // 
//[暂不支持] constexpr id_t CatchRetInstId       = 0x00'00'00'09 | TermInstMask; // 
//[暂不支持] constexpr id_t CatchSwitchInstId    = 0x00'00'00'0A | TermInstMask; // 

constexpr id_t FNegOpInstId         = 0x00'00'00'01 | UnaryOpInstMask; // 负数运算

constexpr id_t AddOpInstId          = 0x00'00'00'01 | BinaryOpInstMask; // 加法运算
constexpr id_t FAddOpInstId         = 0x00'00'00'02 | BinaryOpInstMask; // 浮点加法运算
constexpr id_t SubOpInstId          = 0x00'00'00'03 | BinaryOpInstMask; // 减法运算
constexpr id_t FSubOpInstId         = 0x00'00'00'04 | BinaryOpInstMask; // 浮点减法运算
constexpr id_t MulOpInstId          = 0x00'00'00'05 | BinaryOpInstMask; // 乘法运算
constexpr id_t FMulOpInstId         = 0x00'00'00'06 | BinaryOpInstMask; // 浮点乘法运算
constexpr id_t UDivOpInstId         = 0x00'00'00'07 | BinaryOpInstMask; // 无符号除法运算
constexpr id_t SDivOpInstId         = 0x00'00'00'08 | BinaryOpInstMask; // 有符号除法运算
constexpr id_t FDivOpInstId         = 0x00'00'00'09 | BinaryOpInstMask; // 浮点除法运算
constexpr id_t URemOpInstId         = 0x00'00'00'0A | BinaryOpInstMask; // 无符号求余数运算
constexpr id_t SRemOpInstId         = 0x00'00'00'0B | BinaryOpInstMask; // 有符号求余数运算
constexpr id_t FRemOpInstId         = 0x00'00'00'0C | BinaryOpInstMask; // 浮点求余数运算

constexpr id_t ShlOpInstId          = 0x00'00'00'0D | BinaryOpInstMask; // 左移运算
constexpr id_t LShrOpInstId         = 0x00'00'00'0E | BinaryOpInstMask; // 右移运算
constexpr id_t AShrOpInstId         = 0x00'00'00'0F | BinaryOpInstMask; // 算术右移运算
constexpr id_t AndOpInstId          = 0x00'00'00'10 | BinaryOpInstMask; // 与运算
constexpr id_t OrOpInstId           = 0x00'00'00'11 | BinaryOpInstMask; // 或运算
constexpr id_t XorOpInstId          = 0x00'00'00'12 | BinaryOpInstMask; // 异或运算

constexpr id_t AllocaInstId         = 0x00'00'00'01 | MemoryOpInstMask;
constexpr id_t LoadInstId           = 0x00'00'00'02 | MemoryOpInstMask;
constexpr id_t StoreInstId          = 0x00'00'00'03 | MemoryOpInstMask;
constexpr id_t IndexInstId          = 0x00'00'00'04 | MemoryOpInstMask;
constexpr id_t FenceInstId          = 0x00'00'00'05 | MemoryOpInstMask;
constexpr id_t AtomicCmpXchgInstId  = 0x00'00'00'06 | MemoryOpInstMask;
constexpr id_t AtomicRMWInstId      = 0x00'00'00'07 | MemoryOpInstMask;

constexpr id_t TruncInstId          = 0x00'00'00'01 | CastOpInstMask; // 截断指令
constexpr id_t ZExtInstId           = 0x00'00'00'02 | CastOpInstMask; // 零扩展指令
constexpr id_t SExtInstId           = 0x00'00'00'03 | CastOpInstMask; // 带符号扩展指令
constexpr id_t FPToUIInstId         = 0x00'00'00'04 | CastOpInstMask; // 浮点数->无符号整数
constexpr id_t FPToSIInstId         = 0x00'00'00'05 | CastOpInstMask; // 浮点数->带符号位整数
constexpr id_t UIToFPInstId         = 0x00'00'00'06 | CastOpInstMask; // 无符号整数->浮点数
constexpr id_t SIToFPInstId         = 0x00'00'00'07 | CastOpInstMask; // 带符号位整数->浮点数
constexpr id_t FPTruncInstId        = 0x00'00'00'08 | CastOpInstMask; // 浮点数截断指令
constexpr id_t FPExtInstId          = 0x00'00'00'09 | CastOpInstMask; // 浮点数扩展指令
constexpr id_t PtrToIntInstId       = 0x00'00'00'0A | CastOpInstMask; // 指针->整数
constexpr id_t IntToPtrInstId       = 0x00'00'00'0B | CastOpInstMask; // 整数->指针
constexpr id_t BitCastInstId        = 0x00'00'00'0C | CastOpInstMask; // 不进行任何位变化，仅重新解释
// constexpr id_t AddrSpaceCastInstId  = 0x00'00'00'0D | CastOpInstMask; // 

//[暂不支持] CleanupPad
//[暂不支持] CatchPad

constexpr id_t ICmp                 = 0x00'00'00'01 | OtherInstMask; // 整数比较
constexpr id_t FCmp                 = 0x00'00'00'02 | OtherInstMask; // 浮点数比较
// constexpr id_t PHI                  = 0x00'00'00'03 | OtherInstMask; // phi节点
constexpr id_t Call                 = 0x00'00'00'04 | OtherInstMask; // 函数调用
// constexpr id_t Select               = 0x00'00'00'05 | OtherInstMask; // 
// constexpr id_t UserOp1              = 0x00'00'00'06 | OtherInstMask; // 
// constexpr id_t UserOp2              = 0x00'00'00'07 | OtherInstMask; // 
// constexpr id_t VAArg                = 0x00'00'00'08 | OtherInstMask; // 
// constexpr id_t ExtractElement       = 0x00'00'00'09 | OtherInstMask; // 
// constexpr id_t InsertElement        = 0x00'00'00'0A | OtherInstMask; // 
// constexpr id_t ShuffleVector        = 0x00'00'00'0B | OtherInstMask; // 
// constexpr id_t ExtractValue         = 0x00'00'00'0C | OtherInstMask; // 
// constexpr id_t InsertValue          = 0x00'00'00'0D | OtherInstMask; // 
// constexpr id_t LandingPad           = 0x00'00'00'0E | OtherInstMask; // 

}

#endif