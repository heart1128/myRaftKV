// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: skipListDump.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_skipListDump_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_skipListDump_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3012000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3012004 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_skipListDump_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_skipListDump_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxillaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[1]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_skipListDump_2eproto;
namespace SkipListDumpSerialization {
class KVDump;
class KVDumpDefaultTypeInternal;
extern KVDumpDefaultTypeInternal _KVDump_default_instance_;
}  // namespace SkipListDumpSerialization
PROTOBUF_NAMESPACE_OPEN
template<> ::SkipListDumpSerialization::KVDump* Arena::CreateMaybeMessage<::SkipListDumpSerialization::KVDump>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace SkipListDumpSerialization {

// ===================================================================

class KVDump PROTOBUF_FINAL :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:SkipListDumpSerialization.KVDump) */ {
 public:
  inline KVDump() : KVDump(nullptr) {};
  virtual ~KVDump();

  KVDump(const KVDump& from);
  KVDump(KVDump&& from) noexcept
    : KVDump() {
    *this = ::std::move(from);
  }

  inline KVDump& operator=(const KVDump& from) {
    CopyFrom(from);
    return *this;
  }
  inline KVDump& operator=(KVDump&& from) noexcept {
    if (GetArena() == from.GetArena()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const KVDump& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const KVDump* internal_default_instance() {
    return reinterpret_cast<const KVDump*>(
               &_KVDump_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(KVDump& a, KVDump& b) {
    a.Swap(&b);
  }
  inline void Swap(KVDump* other) {
    if (other == this) return;
    if (GetArena() == other->GetArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(KVDump* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline KVDump* New() const final {
    return CreateMaybeMessage<KVDump>(nullptr);
  }

  KVDump* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<KVDump>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const KVDump& from);
  void MergeFrom(const KVDump& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(KVDump* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "SkipListDumpSerialization.KVDump";
  }
  protected:
  explicit KVDump(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_skipListDump_2eproto);
    return ::descriptor_table_skipListDump_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kKeyFieldNumber = 1,
    kValueFieldNumber = 2,
  };
  // repeated bytes Key = 1;
  int key_size() const;
  private:
  int _internal_key_size() const;
  public:
  void clear_key();
  const std::string& key(int index) const;
  std::string* mutable_key(int index);
  void set_key(int index, const std::string& value);
  void set_key(int index, std::string&& value);
  void set_key(int index, const char* value);
  void set_key(int index, const void* value, size_t size);
  std::string* add_key();
  void add_key(const std::string& value);
  void add_key(std::string&& value);
  void add_key(const char* value);
  void add_key(const void* value, size_t size);
  const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>& key() const;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>* mutable_key();
  private:
  const std::string& _internal_key(int index) const;
  std::string* _internal_add_key();
  public:

  // repeated bytes Value = 2;
  int value_size() const;
  private:
  int _internal_value_size() const;
  public:
  void clear_value();
  const std::string& value(int index) const;
  std::string* mutable_value(int index);
  void set_value(int index, const std::string& value);
  void set_value(int index, std::string&& value);
  void set_value(int index, const char* value);
  void set_value(int index, const void* value, size_t size);
  std::string* add_value();
  void add_value(const std::string& value);
  void add_value(std::string&& value);
  void add_value(const char* value);
  void add_value(const void* value, size_t size);
  const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>& value() const;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>* mutable_value();
  private:
  const std::string& _internal_value(int index) const;
  std::string* _internal_add_value();
  public:

  // @@protoc_insertion_point(class_scope:SkipListDumpSerialization.KVDump)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string> key_;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string> value_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_skipListDump_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// KVDump

// repeated bytes Key = 1;
inline int KVDump::_internal_key_size() const {
  return key_.size();
}
inline int KVDump::key_size() const {
  return _internal_key_size();
}
inline void KVDump::clear_key() {
  key_.Clear();
}
inline std::string* KVDump::add_key() {
  // @@protoc_insertion_point(field_add_mutable:SkipListDumpSerialization.KVDump.Key)
  return _internal_add_key();
}
inline const std::string& KVDump::_internal_key(int index) const {
  return key_.Get(index);
}
inline const std::string& KVDump::key(int index) const {
  // @@protoc_insertion_point(field_get:SkipListDumpSerialization.KVDump.Key)
  return _internal_key(index);
}
inline std::string* KVDump::mutable_key(int index) {
  // @@protoc_insertion_point(field_mutable:SkipListDumpSerialization.KVDump.Key)
  return key_.Mutable(index);
}
inline void KVDump::set_key(int index, const std::string& value) {
  // @@protoc_insertion_point(field_set:SkipListDumpSerialization.KVDump.Key)
  key_.Mutable(index)->assign(value);
}
inline void KVDump::set_key(int index, std::string&& value) {
  // @@protoc_insertion_point(field_set:SkipListDumpSerialization.KVDump.Key)
  key_.Mutable(index)->assign(std::move(value));
}
inline void KVDump::set_key(int index, const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  key_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set_char:SkipListDumpSerialization.KVDump.Key)
}
inline void KVDump::set_key(int index, const void* value, size_t size) {
  key_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:SkipListDumpSerialization.KVDump.Key)
}
inline std::string* KVDump::_internal_add_key() {
  return key_.Add();
}
inline void KVDump::add_key(const std::string& value) {
  key_.Add()->assign(value);
  // @@protoc_insertion_point(field_add:SkipListDumpSerialization.KVDump.Key)
}
inline void KVDump::add_key(std::string&& value) {
  key_.Add(std::move(value));
  // @@protoc_insertion_point(field_add:SkipListDumpSerialization.KVDump.Key)
}
inline void KVDump::add_key(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  key_.Add()->assign(value);
  // @@protoc_insertion_point(field_add_char:SkipListDumpSerialization.KVDump.Key)
}
inline void KVDump::add_key(const void* value, size_t size) {
  key_.Add()->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_add_pointer:SkipListDumpSerialization.KVDump.Key)
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>&
KVDump::key() const {
  // @@protoc_insertion_point(field_list:SkipListDumpSerialization.KVDump.Key)
  return key_;
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>*
KVDump::mutable_key() {
  // @@protoc_insertion_point(field_mutable_list:SkipListDumpSerialization.KVDump.Key)
  return &key_;
}

// repeated bytes Value = 2;
inline int KVDump::_internal_value_size() const {
  return value_.size();
}
inline int KVDump::value_size() const {
  return _internal_value_size();
}
inline void KVDump::clear_value() {
  value_.Clear();
}
inline std::string* KVDump::add_value() {
  // @@protoc_insertion_point(field_add_mutable:SkipListDumpSerialization.KVDump.Value)
  return _internal_add_value();
}
inline const std::string& KVDump::_internal_value(int index) const {
  return value_.Get(index);
}
inline const std::string& KVDump::value(int index) const {
  // @@protoc_insertion_point(field_get:SkipListDumpSerialization.KVDump.Value)
  return _internal_value(index);
}
inline std::string* KVDump::mutable_value(int index) {
  // @@protoc_insertion_point(field_mutable:SkipListDumpSerialization.KVDump.Value)
  return value_.Mutable(index);
}
inline void KVDump::set_value(int index, const std::string& value) {
  // @@protoc_insertion_point(field_set:SkipListDumpSerialization.KVDump.Value)
  value_.Mutable(index)->assign(value);
}
inline void KVDump::set_value(int index, std::string&& value) {
  // @@protoc_insertion_point(field_set:SkipListDumpSerialization.KVDump.Value)
  value_.Mutable(index)->assign(std::move(value));
}
inline void KVDump::set_value(int index, const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  value_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set_char:SkipListDumpSerialization.KVDump.Value)
}
inline void KVDump::set_value(int index, const void* value, size_t size) {
  value_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:SkipListDumpSerialization.KVDump.Value)
}
inline std::string* KVDump::_internal_add_value() {
  return value_.Add();
}
inline void KVDump::add_value(const std::string& value) {
  value_.Add()->assign(value);
  // @@protoc_insertion_point(field_add:SkipListDumpSerialization.KVDump.Value)
}
inline void KVDump::add_value(std::string&& value) {
  value_.Add(std::move(value));
  // @@protoc_insertion_point(field_add:SkipListDumpSerialization.KVDump.Value)
}
inline void KVDump::add_value(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  value_.Add()->assign(value);
  // @@protoc_insertion_point(field_add_char:SkipListDumpSerialization.KVDump.Value)
}
inline void KVDump::add_value(const void* value, size_t size) {
  value_.Add()->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_add_pointer:SkipListDumpSerialization.KVDump.Value)
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>&
KVDump::value() const {
  // @@protoc_insertion_point(field_list:SkipListDumpSerialization.KVDump.Value)
  return value_;
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>*
KVDump::mutable_value() {
  // @@protoc_insertion_point(field_mutable_list:SkipListDumpSerialization.KVDump.Value)
  return &value_;
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace SkipListDumpSerialization

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_skipListDump_2eproto
