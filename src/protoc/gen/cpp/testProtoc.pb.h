// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: testProtoc.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_testProtoc_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_testProtoc_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3021000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3021012 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/message_lite.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_testProtoc_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_testProtoc_2eproto {
  static const uint32_t offsets[];
};
class testPb;
struct testPbDefaultTypeInternal;
extern testPbDefaultTypeInternal _testPb_default_instance_;
PROTOBUF_NAMESPACE_OPEN
template<> ::testPb* Arena::CreateMaybeMessage<::testPb>(Arena*);
PROTOBUF_NAMESPACE_CLOSE

// ===================================================================

class testPb final :
    public ::PROTOBUF_NAMESPACE_ID::MessageLite /* @@protoc_insertion_point(class_definition:testPb) */ {
 public:
  inline testPb() : testPb(nullptr) {}
  ~testPb() override;
  explicit PROTOBUF_CONSTEXPR testPb(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  testPb(const testPb& from);
  testPb(testPb&& from) noexcept
    : testPb() {
    *this = ::std::move(from);
  }

  inline testPb& operator=(const testPb& from) {
    CopyFrom(from);
    return *this;
  }
  inline testPb& operator=(testPb&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const std::string& unknown_fields() const {
    return _internal_metadata_.unknown_fields<std::string>(::PROTOBUF_NAMESPACE_ID::internal::GetEmptyString);
  }
  inline std::string* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields<std::string>();
  }

  static const testPb& default_instance() {
    return *internal_default_instance();
  }
  static inline const testPb* internal_default_instance() {
    return reinterpret_cast<const testPb*>(
               &_testPb_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(testPb& a, testPb& b) {
    a.Swap(&b);
  }
  inline void Swap(testPb* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(testPb* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  testPb* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<testPb>(arena);
  }
  void CheckTypeAndMergeFrom(const ::PROTOBUF_NAMESPACE_ID::MessageLite& from)  final;
  void CopyFrom(const testPb& from);
  void MergeFrom(const testPb& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _impl_._cached_size_.Get(); }

  private:
  void SharedCtor(::PROTOBUF_NAMESPACE_ID::Arena* arena, bool is_message_owned);
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(testPb* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "testPb";
  }
  protected:
  explicit testPb(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  std::string GetTypeName() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kAFieldNumber = 1,
  };
  // optional uint32 a = 1;
  bool has_a() const;
  private:
  bool _internal_has_a() const;
  public:
  void clear_a();
  uint32_t a() const;
  void set_a(uint32_t value);
  private:
  uint32_t _internal_a() const;
  void _internal_set_a(uint32_t value);
  public:

  // @@protoc_insertion_point(class_scope:testPb)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
    uint32_t a_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_testProtoc_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// testPb

// optional uint32 a = 1;
inline bool testPb::_internal_has_a() const {
  bool value = (_impl_._has_bits_[0] & 0x00000001u) != 0;
  return value;
}
inline bool testPb::has_a() const {
  return _internal_has_a();
}
inline void testPb::clear_a() {
  _impl_.a_ = 0u;
  _impl_._has_bits_[0] &= ~0x00000001u;
}
inline uint32_t testPb::_internal_a() const {
  return _impl_.a_;
}
inline uint32_t testPb::a() const {
  // @@protoc_insertion_point(field_get:testPb.a)
  return _internal_a();
}
inline void testPb::_internal_set_a(uint32_t value) {
  _impl_._has_bits_[0] |= 0x00000001u;
  _impl_.a_ = value;
}
inline void testPb::set_a(uint32_t value) {
  _internal_set_a(value);
  // @@protoc_insertion_point(field_set:testPb.a)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_testProtoc_2eproto