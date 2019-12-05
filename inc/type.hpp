#ifndef __type__
#define __type__

#include <map>

/**
 * @file type.hpp
 * @desc :
 *  为Alioth编程语言的类型系统提供类型ID的定义
 *  Alioth编程语言的数据类型系统被设计分为两个大类，数据类型之间的关系承树状结构
 *  详情请参考Alioth: book of design 3.2.3. Type expression
 *      https://github.com/dn-ezr/alioth-lang/blob/master/doc/Alioth:%20book%20of%20design/3.chapter%20three.md#323-type-expression
 */

namespace alioth {

using typeid_t = unsigned long long;
using typemask_t = unsigned long long;

constexpr typeid_t TypeIdBitH =                0x80'00'00'00'00'00'00'00;
constexpr typeid_t TypeIdBitL =                0x00'00'00'00'00'00'00'01;

constexpr typemask_t DeterminedTypeMask =        (TypeIdBitH >> 0x00);
constexpr typemask_t UndeterminedTypeMask =      (TypeIdBitH >> 0x01);
constexpr typemask_t SimpleTypeMask =            (TypeIdBitH >> 0x02) | DeterminedTypeMask;
constexpr typemask_t CompositeTypeMask =         (TypeIdBitH >> 0x03) | DeterminedTypeMask;
constexpr typemask_t PointerTypeMask =           (TypeIdBitH >> 0x04) | SimpleTypeMask;
constexpr typemask_t BasicTypeMask =             (TypeIdBitH >> 0x05) | SimpleTypeMask;
constexpr typemask_t IntegerTypeMask =           (TypeIdBitH >> 0x06) | BasicTypeMask;
constexpr typemask_t FloatPointTypeMask =        (TypeIdBitH >> 0x07) | BasicTypeMask;
constexpr typemask_t UnsignedIntegerTypeMask =   (TypeIdBitH >> 0x08) | IntegerTypeMask;
constexpr typemask_t SignedIntegerTypeMask =     (TypeIdBitH >> 0x09) | IntegerTypeMask;

constexpr typeid_t UnknownType =               (TypeIdBitL << 0x00) | UndeterminedTypeMask;
constexpr typeid_t NamedType =                 (TypeIdBitL << 0x01) | UndeterminedTypeMask;
constexpr typeid_t ThisClassType =             (TypeIdBitL << 0x02) | UndeterminedTypeMask;
constexpr typeid_t UnsolvableType =            (TypeIdBitL << 0x03) | UndeterminedTypeMask;
constexpr typeid_t StructType =                (TypeIdBitL << 0x04) | CompositeTypeMask;
constexpr typeid_t EntityType =                (TypeIdBitL << 0x05) | CompositeTypeMask;
constexpr typeid_t CallableType =              (TypeIdBitL << 0x06) | CompositeTypeMask;
constexpr typeid_t ConstraintedPointerType =   (TypeIdBitL << 0x07) | PointerTypeMask;
constexpr typeid_t UnconstraintedPointerType = (TypeIdBitL << 0x08) | PointerTypeMask;
constexpr typeid_t NullPointerType =           (TypeIdBitL << 0x09) | PointerTypeMask;
constexpr typeid_t VoidType =                  (TypeIdBitL << 0x0a) | BasicTypeMask;
constexpr typeid_t BooleanType =               (TypeIdBitL << 0x0b) | BasicTypeMask;
constexpr typeid_t Uint8Type =                 (TypeIdBitL << 0x0c) | UnsignedIntegerTypeMask;
constexpr typeid_t Uint16Type =                (TypeIdBitL << 0x0d) | UnsignedIntegerTypeMask;
constexpr typeid_t Uint32Type =                (TypeIdBitL << 0x0e) | UnsignedIntegerTypeMask;
constexpr typeid_t Uint64Type =                (TypeIdBitL << 0x0f) | UnsignedIntegerTypeMask;
constexpr typeid_t Int8Type =                  (TypeIdBitL << 0x10) | SignedIntegerTypeMask;
constexpr typeid_t Int16Type =                 (TypeIdBitL << 0x11) | SignedIntegerTypeMask;
constexpr typeid_t Int32Type =                 (TypeIdBitL << 0x12) | SignedIntegerTypeMask;
constexpr typeid_t Int64Type =                 (TypeIdBitL << 0x13) | SignedIntegerTypeMask;
constexpr typeid_t Float32Type =               (TypeIdBitL << 0x14) | FloatPointTypeMask;
constexpr typeid_t Float64Type =               (TypeIdBitL << 0x15) | FloatPointTypeMask;
constexpr typeid_t EnumType =                  (TypeIdBitL << 0x16) | SimpleTypeMask;

/** basic_type_convert_table[dst][src] = cost */
const std::map<std::tuple<typeid_t,typeid_t>,int> basic_type_convert_table;

}

#endif